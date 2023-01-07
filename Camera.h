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

	void Transform(const Point2f& target) const
	{
		Point2f targetPos{ target };
		Clamp(targetPos);
		glTranslatef(-targetPos.x, -targetPos.y, 0.f);
	}

	void Clamp(Point2f& bottomLeft) const
	{
		// TODO: FIX THE CLAMP METHOD

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

};