#include "pch.h"

//External Includes
#include "Room.h"
#include "Camera.h"
#include "Graph.h"
#include "Texture.h"

#include "Game.h"

//Libraries
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

Game::Game(const Window& window)
	:m_Window{ window }
{
	Initialize();
}

Game::~Game()
{
	Cleanup();
}

void Game::Initialize()
{
	//Pre-Allocate memory for the rooms
	m_Rooms.reserve(m_NumOfRoomsToGen);

	//Initialize all the rooms and add them to the array
	for (int i{}; i < m_NumOfRoomsToGen; ++i)
		m_Rooms.emplace_back(new Room(m_Window.width, m_Window.height, i));

	//Sort the rooms from smallest to largest
	std::sort(m_Rooms.begin(), m_Rooms.end(), Room::CompareRoomSize);

	//Initialize the camera
	m_pCamera = new Camera(m_Window.width, m_Window.height);
	m_pCamera->SetLevelBoundaries(Rectf{ -m_Window.width / 2.f, -m_Window.height / 2.f, m_Window.width / 2.f, m_Window.height / 1.5f});

	//Initialize the graph
	m_pGraph = new Graph();

	PrintControls();
}

void Game::Cleanup()
{
	for (const auto& room : m_Rooms)
		delete room;
	for (const auto& room : m_DeletedRooms)
		delete room;
	delete m_pCamera;
	delete m_pGraph;
}

void Game::Update(float elapsedSec)
{

	HandleDungeonGeneration();
	HandleInput();

	for (const auto& room : m_Rooms)
		room->Update(); //Update the rooms

	UpdateTimer(elapsedSec);
	m_pCamera->Clamp(m_CameraPosition);
}

void Game::Draw() const
{
	ClearBackground();

	//If still generating the old dungeon
	if (m_CurrentStage != done)
	{
		const Texture generatingTexture{ "Generating Dungeon...","Fonts/dogica.ttf" ,30, Color4f{1,1,1,1} };
		generatingTexture.Draw(Rectf{ m_Window.width / 2.f - generatingTexture.GetWidth() / 2.f, 
			m_Window.height / 2.f - generatingTexture.GetHeight() / 2.f,
			generatingTexture.GetWidth(), generatingTexture.GetHeight() });
		return;
	}

	glPushMatrix();
	{
		glScalef(m_ZoomIn, m_ZoomIn, m_ZoomIn);
		//Update camera canvas
		m_pCamera->Transform(m_CameraPosition);

		

		for (const auto& hallway : m_Hallways)
		{
			utils::SetColor(Color4f{ 76.1f, 69.8f, 50.2f ,1.0f });
			utils::DrawLine(hallway.startingPoint, hallway.endPoint, float(hallway.hallwaySize));
		}

		for (const auto& room : m_Rooms)
			room->Draw();

		if (m_DoDebug)
		{
			for (const auto& room : m_DeletedRooms)
			{
				utils::SetColor(Color4f{0,0.5f,0.5f,1});
				utils::DrawRect(room->GetRect());
			}

			m_pGraph->DebugDraw();
			for (const auto& room : m_Rooms)
			{
				const Texture roomID("ID:" + std::to_string(room->GetId()), "Fonts/dogica.ttf", 7, Color4f{1,1,1,1});
				roomID.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom, roomID.GetWidth(), roomID.GetHeight() });

				const Texture roomX("X:" + std::to_string(int(std::round(room->GetPosition().x))), "Fonts/dogica.ttf", 8, Color4f{ 1,1,1,1 });
				roomX.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom + room->GetRect().height - roomX.GetHeight()
					,roomX.GetWidth(), roomX.GetHeight() });

				const Texture roomY("Y:" + std::to_string(int(std::round(room->GetPosition().y))), "Fonts/dogica.ttf", 8, Color4f{ 1,1,1,1 });
				roomY.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom + room->GetRect().height - 3 * roomY.GetHeight()
					,roomY.GetWidth(), roomY.GetHeight() });
			}
		}
	}
	glPopMatrix();

	DrawUI();
}

void Game::DrawUI() const
{
	//Draw timer
	std::stringstream stream{};
	stream << std::fixed << std::setprecision(1) << m_MaxDisplayTime - m_CurrentDisplayTime;
	const Texture timerTexture{ "Next Room In:" + stream.str() , "Fonts/dogica.ttf", 25, Color4f{1,1,1,1} };
	constexpr float windowGap{ 10 };
	timerTexture.Draw(Rectf{ m_Window.width - timerTexture.GetWidth() - windowGap, m_Window.height - timerTexture.GetHeight() - windowGap,
		timerTexture.GetWidth(), timerTexture.GetHeight() });

	//Draw number of rooms
	const Texture roomCounterTexture{ "Minimum Number Of Rooms:" + std::to_string(m_MinimumNumOfRooms),"Fonts/dogica.ttf" ,10, Color4f{1,1,1,1} };
	roomCounterTexture.Draw(Point2f{ windowGap, m_Window.height - roomCounterTexture.GetHeight() - windowGap });
}

void Game::ResetDungeon()
{
	for (const auto& room : m_Rooms)
		delete room;
	for (const auto& room : m_DeletedRooms)
		delete room;

	m_Rooms.clear();
	m_DeletedRooms.clear();
	m_Hallways.clear();
	m_pGraph->Reset();

	//Re-Calculate this as the user might change it
	m_NumOfRoomsToGen = { m_MinimumNumOfRooms * 2 };

	//Pre-Allocate memory for the rooms
	m_Rooms.reserve(m_NumOfRoomsToGen);

	//Initialize all the rooms and add them to the array
	for (int i{}; i < m_NumOfRoomsToGen; ++i)
		m_Rooms.emplace_back(new Room(m_Window.width, m_Window.height, i));

	m_CurrentStage = Game::CurrentStage::roomSeparation;
}

void Game::UpdateTimer(float elapsedSec)
{
	//Update Timer
	if (!m_IsTimerPaused && m_CurrentStage == done)
		m_CurrentDisplayTime += elapsedSec;

	if (m_CurrentDisplayTime >= m_MaxDisplayTime)
	{
		m_CurrentDisplayTime -= m_MaxDisplayTime;
		ResetDungeon();
	}
}

void Game::HandleDungeonGeneration()
{
	switch (m_CurrentStage)
	{
		//Step 1: Separate the rooms
	case Game::roomSeparation:
	{
		float tightnessChecked{ m_RoomTightness };
		if (tightnessChecked < 1.f) tightnessChecked = 1;
		if (tightnessChecked > 3.f) tightnessChecked = 3;

		Room::SeparateRooms(m_Rooms, tightnessChecked);
		if (!Room::AreRoomsOverlapping(m_Rooms)) m_CurrentStage = Game::CurrentStage::roomDeletion;
	}
	break;
	//Step 2: Delete all the secondary rooms
	case Game::roomDeletion:
	{
		for (int i{ m_NumOfRoomsToGen }; i > m_MinimumNumOfRooms; --i)
		{
			m_DeletedRooms.emplace_back(m_Rooms[i - 1]);
			m_Rooms.pop_back();
		}
		std::vector<Vertex> points{};
		for (const auto& room : m_Rooms)
			points.emplace_back(room->GetPosition(), room->GetId());

		m_pGraph->SetPoints(points);
	}
	m_CurrentStage = Game::CurrentStage::delaunyTriangulation;
	break;
	//Step 3: Calculate the Delauny Triangulation
	case Game::delaunyTriangulation:
		m_pGraph->CalculateTriangulation();
		m_CurrentStage = Game::CurrentStage::MST;
		break;
		//Step 4: Find the minimum spanning tree for the triangulation
	case Game::MST:
		m_pGraph->CalculateMST();
		m_CurrentStage = Game::CurrentStage::roomConnections;
		break;
		//Step 5: Randomly add deleted edges to add variation and cycles to the dungeon
	case Game::roomConnections:
		m_pGraph->FillRoomConnections();
		m_CurrentStage = Game::CurrentStage::addingHallways;
		break;
		//Step 6: Connect the rooms based on the room connections formed from the previous steps
	case Game::addingHallways:
		CreateHallways();
		m_CurrentStage = Game::CurrentStage::addDeletedRooms;
		break;
	case Game::addDeletedRooms:
		for (const auto& hallway : m_Hallways)
		{
			float minFlt{}, maxFlt{};
			for (auto it = m_DeletedRooms.begin(); it != m_DeletedRooms.end();)
			{
				auto& room = *it;
				if (utils::IntersectRectLine(room->GetRect(), hallway.startingPoint, hallway.endPoint, minFlt, maxFlt))
				{
					m_Rooms.emplace_back(std::move(room));
					it = m_DeletedRooms.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		m_CurrentStage = Game::CurrentStage::done;

		std::sort(m_Rooms.begin(), m_Rooms.end(), Room::CompareRoomSize);
		//Add any special rooms here:
		m_Rooms.back()->SetSpecialRoom(Room::SpecialRoomTypes::BOSS);
		break;
	}
}

void Game::HandleInput()
{
	const Uint8* pStates = SDL_GetKeyboardState(nullptr);

	//Camera Input
	if (pStates[SDL_SCANCODE_LEFT])
		m_CameraPosition.x -= m_CameraMoveSpeed;
	if (pStates[SDL_SCANCODE_RIGHT])
		m_CameraPosition.x += m_CameraMoveSpeed;
	if (pStates[SDL_SCANCODE_UP])
		m_CameraPosition.y += m_CameraMoveSpeed;
	if (pStates[SDL_SCANCODE_DOWN])
		m_CameraPosition.y -= m_CameraMoveSpeed;
}

void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
	if (e.keysym.sym == SDLK_SPACE)
	{
		m_IsTimerPaused = !m_IsTimerPaused;
	}

	if (e.keysym.sym == SDLK_k)
	{
		++m_MinimumNumOfRooms;
	}
	if (e.keysym.sym == SDLK_j)
	{
		if (--m_MinimumNumOfRooms < 3) m_MinimumNumOfRooms = 3;
	}
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
	
}
void Game::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
	
}
void Game::ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
{
	
}
void Game::ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
{
	
}
void Game::ProcessScrollUpEvent(const SDL_MouseWheelEvent& e)
{
	m_ZoomIn += 0.05f;
}
void Game::ProcessScrollDownEvent(const SDL_MouseWheelEvent& e)
{
	m_ZoomIn -= 0.05f;
}

void Game::ClearBackground() const
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Game::CreateHallways()
{
	std::vector<Connection> MSTEdges{ m_pGraph->GetRoomConnections() };
	Room room1{}, room2{};
	auto VertexToRoom = [&](const Vertex& vertex, const std::vector<Room*> rooms) ->Room
	{
		for (const auto& room : rooms)
		{
			if (vertex.roomConnectionID == room->GetId()) return *room;
		}

		return Room{};
	};

	for (const auto& edge : MSTEdges)
	{
		Room::ConnectRooms(VertexToRoom(edge.start, m_Rooms), VertexToRoom(edge.end, m_Rooms), m_Hallways);
	}
}

void Game::PrintControls() const 
{
	std::cout << "\033[1;33m=================\033[1;31mCONTROLS\033[1;33m================\033[0m" << std::endl;
	std::cout << "Use \033[1;31mSPACE\033[0m to \033[1;32mPause\033[0m the timer" << std::endl;
	std::cout << "Use the \033[1;31mArrow Keys\033[0m to \033[1;32mMove\033[0m around" << std::endl;
	std::cout << "Use the \033[1;31mScroll Wheel\033[0m to \033[1;32mZoom In/Out\033[0m" << std::endl;
	std::cout << "Use \033[1;31mJ/K\033[0m to \033[1;32mDecrease/Increase\033[0m the Rooms" << std::endl;
	std::cout << "\033[1;33m=========================================\033[0m" << std::endl;
}
