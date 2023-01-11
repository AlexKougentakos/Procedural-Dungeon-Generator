#pragma once
#include "pch.h"

class Camera
{
public:
	Camera(float width, float height)
	{
		
	}

	~Camera()
	{
		
	}

	void Transform(const Point2f& target)
	{
		Point2f targetPos{ target };
		Clamp(targetPos);
		m_ClampedPoint = targetPos;
		glTranslatef(-targetPos.x, -targetPos.y, 0.f);
	}

	Point2f GetClampedTarget() { return m_ClampedPoint; }

	void Clamp(Point2f& bottomLeft)
	{
		if (bottomLeft.x < m_LevelBoundaries.left)
		{
			bottomLeft.x = m_LevelBoundaries.left;
		}

		if (bottomLeft.y < m_LevelBoundaries.bottom)
		{
			bottomLeft.y = m_LevelBoundaries.bottom;
		}

		if (bottomLeft.x + m_Width >= m_LevelBoundaries.width)
		{
			bottomLeft.x = m_LevelBoundaries.width - m_Width;
		}

		if (bottomLeft.y + m_Height >= m_LevelBoundaries.height)
		{
			bottomLeft.y = m_LevelBoundaries.height - m_Height;
		}
	}

	void SetLevelBoundaries(const Rectf& levelBoundaries) { m_LevelBoundaries = levelBoundaries; }

private:
	Rectf m_LevelBoundaries{};
	float m_Width{};
	float m_Height{};

	Point2f m_ClampedPoint{};

};