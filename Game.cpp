#include "pch.h"

//External Includes
#include "Room.h"
#include "Camera.h"
#include "Graph.h"

#include "Game.h"

//Libraries
#include <algorithm>
#include <iostream>
#include <map>

Game::Game( const Window& window ) 
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
	m_pCamera->SetLevelBoundaries(Rectf{ -m_Window.width, -m_Window.height, 2 * m_Window.width, 2 * m_Window.height });

	//Initialize the graph
	m_pGraph = new Graph();
}

void Game::Cleanup( )
{
	for (const auto& room : m_Rooms)
		delete room;
	for (const auto& room : m_DeletedRooms)
		delete room;
	delete m_pCamera;
	delete m_pGraph;
}

void Game::Update( float elapsedSec )
{
	// Check keyboard state
	//const Uint8 *pStates = SDL_GetKeyboardState( nullptr );
	//if ( pStates[SDL_SCANCODE_RIGHT] )
	//{
	//	std::cout << "Right arrow key is down\n";
	//}
	//if ( pStates[SDL_SCANCODE_LEFT] && pStates[SDL_SCANCODE_UP])
	//{
	//	std::cout << "Left and up arrow keys are down\n";
	//}

	switch (m_CurrentStage)
	{
		//Step 1: Separate the rooms
	case Game::roomSeparation:
		Room::SeparateRooms(m_Rooms);
		if (!Room::AreRoomsOverlapping(m_Rooms)) m_CurrentStage = Game::CurrentStage::roomDeletion;
		break;
		//Step 2: Delete all the secondary rooms
	case Game::roomDeletion:
	{
		for (int i{m_NumOfRoomsToGen}; i > m_NumOfRooms; --i)
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
					room->SetIsSecondary(true);
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
		break;
	}


	HandleInput();
	for (const auto& room : m_Rooms)
		room->Update(); //Update the rooms
}

void Game::Draw( ) const
{
	ClearBackground( );
	if (m_CurrentStage != done) return;
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
				utils::SetColor(colors::cyan);
				utils::DrawRect(room->GetRect());
			}

			m_pGraph->DebugDraw();
			for (const auto& room : m_Rooms)
			{
				const Texture roomID("ID:" + std::to_string(room->GetId()), "Fonts/dogica.ttf", 7, colors::white);
				roomID.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom, roomID.GetWidth(), roomID.GetHeight() });

				const Texture roomX("X:" + std::to_string( int(std::round(room->GetPosition().x))), "Fonts/dogica.ttf", 8, colors::white);
				roomX.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom + room->GetRect().height - roomX.GetHeight()
					,roomX.GetWidth(), roomX.GetHeight() });

				const Texture roomY("Y:" + std::to_string(int(std::round(room->GetPosition().y))), "Fonts/dogica.ttf", 8, colors::white);
				roomY.Draw(Rectf{ room->GetRect().left, room->GetRect().bottom + room->GetRect().height - 3 * roomY.GetHeight()
					,roomY.GetWidth(), roomY.GetHeight() });
			}
		}


	}
	glPopMatrix();
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


void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{

}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{

}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
	if (m_IsMouseDown)
	{
		//todo add mouse movable camera
		//m_CameraPosition = Point2f{ float(e.x), float(e.y) };
	}
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	m_IsMouseDown = true;
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
	m_IsMouseDown = false;
}
void Game::ProcessScrollUpEvent(const SDL_MouseWheelEvent& e)
{
	m_ZoomIn += 0.05f;
}
void Game::ProcessScrollDownEvent(const SDL_MouseWheelEvent& e)
{
	m_ZoomIn -= 0.05f;
}


void Game::ClearBackground( ) const
{
	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
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
