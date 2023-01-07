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

	bool didSeparate{ false };
	for (int i{0}; i < m_Rooms.size(); ++i)
		for (int j{0}; j < m_Rooms.size(); ++j)
		{
			//Check if room is overlapping with itself.
			if (m_Rooms[i] == m_Rooms[j]) continue;
			didSeparate = Room::SeparateRooms(*m_Rooms[i], *m_Rooms[j]);
		}

	if (didSeparate && !m_DidDelete)
	{
		for (int i{m_NumOfRoomsToGen}; i > m_NumOfRooms; --i)
		{
			delete m_Rooms[i-1];
			m_Rooms.pop_back();
		}
		m_DidDelete = true;
	}

	HandleInput();
	for (const auto& room : m_Rooms)
		room->Update(); //Update the rooms
}

void Game::Draw( ) const
{
	ClearBackground( );

	glPushMatrix();
	{
		glScalef(m_ZoomIn, m_ZoomIn, m_ZoomIn);
		//Update camera canvas
		m_pCamera->Transform(m_CameraPosition);

		for (const auto& room : m_Rooms)
			room->Draw();

		if (m_DoDebug)
		{
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

	//todo add these manually
	if (e.keysym.sym == SDLK_k)
	{
		m_pGraph->CalculateTriangulation();
	}	if (e.keysym.sym == SDLK_j)
	{
		std::vector<Vertex> verticesIn;
		std::vector<Vertex> verticesOut;
		for (const auto& room : m_Rooms)
			verticesIn.push_back(Vertex{ room->GetPosition().x, room->GetPosition().y });
		m_pGraph->SetPoints(verticesIn, verticesOut);
	}
	if (e.keysym.sym == SDLK_l)
	{
		m_pGraph->CalculateMST();
	}
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{

}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
	if (m_IsMouseDown)
	{
		m_CameraPosition = Point2f{ float(e.x), float(e.y) };
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
