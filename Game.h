#pragma once

class Room;
class Camera;
class Graph;

struct Hallway;

class Game final
{
public:
	explicit Game(const Window& window);
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game(Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	~Game();

	void Update(float elapsedSec);
	void DrawUI() const;
	void Draw() const;

	// Event handling
	void ProcessKeyDownEvent(const SDL_KeyboardEvent& e);
	void ProcessKeyUpEvent(const SDL_KeyboardEvent& e);
	void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e);
	void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e);
	void ProcessMouseUpEvent(const SDL_MouseButtonEvent& e);
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
	int m_MinimumNumOfRooms{ 10 };
	float m_RoomTightness{1.f}; //[1,3]

	//Hidden Settings
	int m_NumOfRoomsToGen{ m_MinimumNumOfRooms * 2 };
	CurrentStage m_CurrentStage{ roomSeparation };
	const int m_CameraMoveSpeed{ 10 };

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
	bool m_DidDelete{ false };
	bool m_DoDebug{ false };
	bool m_DidSetPoints{ false };
	bool m_IsTimerPaused{ false };

	//Timer Variables
	float m_MaxDisplayTime{ 5.f }; //In Seconds
	float m_CurrentDisplayTime{ 0.f };


	// FUNCTIONS
	void Initialize();
	void Cleanup();
	void ClearBackground() const;

	void HandleInput();
	void CreateHallways();
	void ResetDungeon();
	void HandleDungeonGeneration();
	void UpdateTimer(float elapsedSec);
	void PrintControls() const;
};