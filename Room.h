#pragma once
#include "Texture.h"
#include  "Definitions.h"
#include "pch.h"

#define RANDOM_POSITION

class Room
{
public:
	Room() = default;
	Room(float winWidth, float winHeight, int ID)
		:m_WindowWidth{ winWidth },
		m_WindowHeight{ winHeight },
		m_RoomID{ID}
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
		utils::SetColor(m_Colour);
		utils::DrawRect(m_Rect);
		utils::FillEllipse(GetPosition(), 3, 3);		
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

	static bool SeparateRooms(Room& r1, Room& r2)
	{
		if ( utils::IsOverlapping(r1.GetRect(), r2.GetRect()) )
		{
			constexpr int moveIncrement{ 3 };

			Vector2f smallToBigVector{};
			Vector2f smallToBigNormalized{};

			Vector2f bigToSmallNormalized{};

			if (r1.GetArea() > r2.GetArea())
			{
				smallToBigVector = r1.GetPosition() - r2.GetPosition();
				smallToBigNormalized = smallToBigVector.Normalized();

				bigToSmallNormalized = -smallToBigNormalized;
			}
			else
			{
				smallToBigVector =  r2.GetPosition() - r1.GetPosition();
				smallToBigNormalized = smallToBigVector.Normalized();

				bigToSmallNormalized = -smallToBigNormalized;
			}

			if (r1.GetArea() >= r2.GetArea())
			{
				r1.m_Position.x += moveIncrement * smallToBigNormalized.x;
				r1.m_Position.y += moveIncrement * smallToBigNormalized.y;

				r2.m_Position.x += moveIncrement * bigToSmallNormalized.x;
				r2.m_Position.y += moveIncrement * bigToSmallNormalized.y;
			}

			return false;
		}
		return true;
	}

	static bool AreRoomsOverlapping(Room& r1, Room& r2)
	{
		return utils::IsOverlapping(r1.GetRect(), r2.GetRect());
	}

	static void ConnectRooms(const Room& r1, const Room& r2)
	{
		
	}

	static bool CompareRoomSize(const Room* a, const Room* b)
	{
		return a->GetArea() < b->GetArea();
	}


private:
	Color4f m_Colour{};
	Rectf m_Rect{};
	Point2f m_Position{};
	float m_Area{};
	int m_RoomID{};

	const int m_MinSize{ 30 }, m_MaxSize{ 80 };
	const int m_WindowBorderEdgeGap{ 20 };
	const float m_WindowWidth{}, m_WindowHeight{};
};
