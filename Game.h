#pragma once

class Room;
class Camera;
class Graph;

struct Hallway;

class Game final
{
public:
	explicit Game( const Window& window );
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game( Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	~Game();

	void Update( float elapsedSec );
	void Draw( ) const;

	// Event handling
	void ProcessKeyDownEvent( const SDL_KeyboardEvent& e );
	void ProcessKeyUpEvent( const SDL_KeyboardEvent& e );
	void ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e );
	void ProcessMouseDownEvent( const SDL_MouseButtonEvent& e );
	void ProcessMouseUpEvent( const SDL_MouseButtonEvent& e );
	void ProcessScrollUpEvent(const SDL_MouseWheelEvent& e);
	void ProcessScrollDownEvent(const SDL_MouseWheelEvent& e);

private:
	enum CurrentStage
	{
		roomSeparation,
		roomDeletion,
		delaunyTriangulation,
		MST,
		roomConnections,
		addingHallways,
		addDeletedRooms,
		done
	};

	// DATA MEMBERS
	const Window m_Window;

	//Settings
	const int m_NumOfRooms{ 10 };
	const int m_CameraMoveSpeed{ 10 };

	//Hidden Settings
	const int m_NumOfRoomsToGen{ m_NumOfRooms * 2 };
	CurrentStage m_CurrentStage{ roomSeparation };

	//Class/Struct Instances
	std::vector<Room*> m_Rooms{};
	std::vector<Room*> m_DeletedRooms{};
	Camera* m_pCamera{};
	Graph* m_pGraph{};
	std::vector<Hallway> m_Hallways{};

	//Camera Variables
	Point2f m_CameraPosition{ 0.f, 0.f };
	Point2f m_Origin{};
	Point2f m_Difference{};
	Point2f m_ResetCamera{};
	float m_ZoomIn{ 1.f };

	//Other Variables
	bool m_IsMouseDown{ false };
	bool m_DidDelete{ false };
	bool m_DoDebug{ false };
	bool m_DidSetPoints{ false };


	// FUNCTIONS
	void Initialize( );
	void Cleanup( );
	void ClearBackground( ) const;

	void HandleInput();
	void CreateHallways();
};