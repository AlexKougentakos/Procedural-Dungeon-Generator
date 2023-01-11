#pragma once
#include "Texture.h"
#include "pch.h"

#define RANDOM_POSITION

struct Hallway
{
	Hallway()
	{
		startingPoint = Point2f{};
		endPoint = Point2f{};
	}
	Hallway(const Point2f& start, const Point2f& end)
	{
		startingPoint = start;
		endPoint = end;
	}

	bool operator==(const Hallway& hallway)
	{
		return (Vector2f{ startingPoint } == Vector2f{ hallway.startingPoint } && Vector2f{ endPoint } == Vector2f{ hallway.endPoint });
	}

	Point2f startingPoint;
	Point2f endPoint;
	int hallwaySize{ 6 };
};

class Room
{
public:
	Room() = default;
	Room(float winWidth, float winHeight, int ID)
		:m_WindowWidth{ winWidth },
		m_WindowHeight{ winHeight },
		m_RoomID{ID},
		m_RoomType{DEFAULT}
	{
		Initialize();
	}
	~Room() = default;

	Room operator=(const Room& room) { return *this; }

	void Initialize()
	{
		m_Colour = Color4f{ float(rand() % 101) / 100, float(rand() % 101) / 100,
		float(rand() % 101) / 100 ,float(rand() % 101) / 100 };
		const float width{ float((rand() % m_MinSize) + m_MaxSize - m_MinSize) };
		const float height{ float((rand() % m_MinSize) + m_MaxSize - m_MinSize) };

		m_Area = width * height;

#ifdef RANDOM_POSITION

		const float centerY{ (m_WindowHeight / 2.f) + m_Rect.height / 2.f};
		const float randY{ float(rand() % int(centerY - 10) - 20) };

		m_Position = Point2f{ float(rand() % int(m_WindowWidth)), randY };
#else
		m_Position.x = m_WindowWidth;
		m_Position.y = m_WindowHeight;
#endif

		//todo add height above centerY to make them spawn at the center.
		m_Rect = Rectf{ m_Position.x, m_Position.y, width, height };
	}

	void Draw() const
	{

		utils::SetColor(Color4f{0.152f, 0.15f, 0.15f, 1.f});
		utils::FillRect(m_Rect);
		if (m_RoomType == BOSS)
		{
			const Texture bossIconTexture{ "Assets/boss_icon.png" };
			bossIconTexture.Draw(Rectf{m_Rect.left + m_Rect.width / 2.f - bossIconTexture.GetWidth() / 2.f, 
				m_Rect.bottom + m_Rect.height / 2.f - bossIconTexture.GetHeight()	/ 2.f,
				bossIconTexture.GetWidth(), bossIconTexture.GetHeight()});
		}

		utils::SetColor(Color4f{ 1,1,1,1 });
		utils::DrawRect(Rectf{ m_Rect.left + m_OutlineThickness / 2.f, m_Rect.bottom + m_OutlineThickness / 2.f ,
			m_Rect.width - m_OutlineThickness / 2.f , m_Rect.height - m_OutlineThickness / 2.f }, m_OutlineThickness / 2.f);
	}

	void Update()
	{
		//Make the rectangle dependant on the position
		m_Rect.left = m_Position.x;
		m_Rect.bottom = m_Position.y;
	}

	//Helper Functions
	float GetArea() const { return m_Area; }
	//Returns the center of the room
	Point2f GetPosition() const
	{
		return Point2f{ m_Rect.left + m_Rect.width / 2.f, m_Rect.bottom + m_Rect.height / 2.f };
	}
	Rectf GetRect() const { return m_Rect; }

	int GetId() const { return  m_RoomID; }

	static void FindBiggestRoom(const Room& r1, const Room& r2, Room& roomOut)
	{
		if (r1.GetArea() >= r2.GetArea()) roomOut = r1;
		else roomOut = r2;
	}

	static void SeparateRooms(std::vector<Room*> rooms, float roomTightness)
	{
		constexpr float roomGap{ 15 };
		const float fleeRange{ roomTightness * m_MaxSize + roomGap};
		constexpr float fleeSpeed{ 10 };

		for (const auto& room : rooms)
		{
			for (const auto& roomToEvade : rooms)
			{
				if (room == roomToEvade) continue;
				Vector2f fleeVector = Vector2f {room->GetPosition() }- Vector2f{roomToEvade->GetPosition()};
				const float distance{ fleeVector.Length() };
				if (distance < fleeRange)
				{
					const Vector2f fleeVectorNormal = fleeVector.Normalized();

					/*fleeVector *= fleeSpeed; //This option scatters them more
					room->m_Position += fleeVector;*/
					room->m_Position += fleeVectorNormal * fleeSpeed;
					room->m_IsFleeing = true;
				}
				else
				{
					room->m_IsFleeing = false;
				}
			}
		}
	}

	static bool AreRoomsOverlapping(std::vector<Room*> rooms)
	{
		for (const auto& room : rooms)
		{
			for (const auto& roomToCompare : rooms)
			{
				if (room == roomToCompare) continue;
				if (utils::IsOverlapping(room->GetRect(), roomToCompare->GetRect()))
					return true;
			}
		}
		return false;
	}

	static void ConnectRooms(const Room& r1, const Room& r2, std::vector<Hallway>& hallways)
	{
		const float smallestX = std::min(r1.GetPosition().x, r2.GetPosition().x);
		const float smallestY = std::min(r1.GetPosition().y, r2.GetPosition().y);
		const float biggestX = std::max(r1.GetPosition().x, r2.GetPosition().x);
		const float biggestY = std::max(r1.GetPosition().y, r2.GetPosition().y);

		Point2f commonPointBottom{ biggestX, smallestY };
		Point2f commonPointTop{ smallestX, biggestY };
		Point2f commonPoint{};

		//Make sure the common point is not a room, if it is then offset it to the other diagonal
		if (Vector2f{ r1.GetPosition() } == Vector2f{ biggestX, smallestY } ||
			Vector2f{ r1.GetPosition() } == Vector2f{ smallestX, biggestY })
		{
			commonPointBottom = Point2f{ smallestX, smallestY };
			commonPointTop = Point2f{ biggestX, biggestY };
		}

		//Make it random weather the rooms will connect from the top or the bottom, since both ways are equally long
		const int chance{ rand() % 101 };
		if (chance > 50) commonPoint = commonPointBottom;
		else commonPoint = commonPointTop;

		if (smallestY == biggestY || smallestX == biggestX)
		{
			Hallway hallway{ Point2f{smallestX, smallestY}, Point2f{biggestX, biggestY} };
			for (const auto& hallwayToCheck : hallways)
				if (hallway == hallwayToCheck)
					return;
			hallways.emplace_back(hallway);
		}
		else
		{
			Hallway hallway1{ Point2f{r1.GetPosition().x, r1.GetPosition().y}, commonPoint };
			Hallway hallway2{ Point2f{r2.GetPosition().x, r2.GetPosition().y}, commonPoint };

			for (const auto& hallwayToCheck : hallways)
				if (hallway1 == hallwayToCheck || hallway2 == hallwayToCheck)
					return;

			hallways.emplace_back(hallway1);
			hallways.emplace_back(hallway2);
		}
	}

	static bool CompareRoomSize(const Room* a, const Room* b)
	{
		return a->GetArea() < b->GetArea();
	}

	bool GetIsFleeing() const { return m_IsFleeing; }

	enum SpecialRoomTypes
	{
		DEFAULT,
		BOSS
	};

	SpecialRoomTypes GetRoomType() const { return m_RoomType; }
	void SetSpecialRoom(SpecialRoomTypes type) { m_RoomType = type; }

private:
	Color4f m_Colour{};
	Rectf m_Rect{};
	Point2f m_Position{};
	float m_Area{};
	int m_RoomID{};
	float m_OutlineThickness{ m_MinSize / 9.f };
	bool m_IsFleeing{ false };

	static constexpr int m_MinSize{ 30 }, m_MaxSize{ 80 };
	const int m_WindowBorderEdgeGap{ 20 };
	const float m_WindowWidth{}, m_WindowHeight{};

	SpecialRoomTypes m_RoomType{DEFAULT};
};
