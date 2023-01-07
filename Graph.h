#pragma once
#include <cmath>
#include "pch.h"

#include <algorithm>

#include "MathHelpers.h"
#include "Definitions.h"
#include  "Texture.h"
#include <map>

#include <iostream>
#include <unordered_map>

#define INVALID_MST_GROUP 0

struct Vertex
{
	float x;
	float y;
	int MSTGroup{ INVALID_MST_GROUP };

	//Operator Overloading
	Vertex operator-(const Vertex& vertex) const
	{
		return { x - vertex.x, y - vertex.y };
	}
	Vertex& operator-=(const Vertex& vertex)
	{
		x -= vertex.x;
		y -= vertex.y;

		return *this;
	}
	Vertex operator+(const Vertex& vertex) const
	{
		return { x + vertex.x, y + vertex.y };
	}
	Vertex& operator+=(const Vertex& vertex)
	{
		x += vertex.x;
		y += vertex.y;

		return *this;
	}
	bool operator==(const Vertex& vertex) const
	{
		return (x == vertex.x && y == vertex.y);
	}
	bool operator!=(const Vertex& vertex) const
	{
		return (x != vertex.x || y != vertex.y);
	}

};

struct Connection
{
	Connection()
	{
		start = {};
		end = {};

		weight = {};
	}
	Connection(Vertex _start, Vertex _end)
	{
		start = _start;
		end = _end;

		weight =sqrtf(powf(end.x - start.x, 2) + powf(end.y - start.y, 2));
	}

	Vertex start;
	Vertex end;
	float weight;

	Connection(const Connection& connection)
	{
		start = connection.start;
		end = connection.end;
		weight = connection.weight;
	}
	bool operator==(const Connection& connection) const
	{
		return (start == connection.start && end == connection.end);
	}
	bool operator<(const Connection& connection) const
	{
		return (weight < connection.weight);
	}
};


struct Triangle
{
public:
	Triangle()
{
	a = { 0,0 };
	b = { 0,0 };
	c = { 0,0 };
	circumCircle = {};

	ab = { a,b };
	bc = { b,c };
	ac = { a,c };
}
	Triangle(Vertex _a, Vertex _b, Vertex _c)
	{
		a = _a;
		b = _b;
		c = _c;
		circumCircle = calculateCircum();

		ab = { a,b };
		bc = { b,c };
		ac = { a,c };

		edges.push_back(ab);
		edges.push_back(ac);
		edges.push_back(bc);
	}

	bool operator==(const Triangle& triangle) const
	{
		return (a == triangle.a && b == triangle.b && c == triangle.c);
	}
	bool operator!=(const Triangle& triangle) const
	{
		return (a != triangle.a || b != triangle.b || c != triangle.c);
	}

	Vertex a;
	Vertex b;
	Vertex c;

	std::vector<Connection> edges;

	Circlef circumCircle{ };

private:
	//Connections
	Connection ab;
	Connection bc;
	Connection ac;

	Circlef calculateCircum()
	{
		//Calculate side lengths
		const float ab{ sqrtf(powf(b.x - a.x, 2.f) + powf(b.y - a.y, 2.f)) };
		const float ac{ sqrtf(powf(c.x - a.x, 2.f) + powf(c.y - a.y, 2.f)) };
		const float cb{ sqrtf(powf(b.x - c.x, 2.f) + powf(b.y - c.y, 2.f)) };

		//Calculate Semi-Perimeter
		const float semiPerimeter{ (ab + ac + cb) / 2.f };

		//Calculate Area
		const float area{ sqrtf(semiPerimeter * (semiPerimeter - ab) * (semiPerimeter - ac) * (semiPerimeter - cb)) };

		//Calculate Circle Radius
		const float radius{ (ab * ac * cb) / (4.f * area) };

		//Calculate Circle Center
		const Point2f center{ ((b.y - a.y) * (c.y * c.y - a.y * a.y + c.x * c.x - a.x * a.x) - (c.y - a.y) * (b.y * b.y - a.y * a.y + b.x * b.x - a.x * a.x)) /
			(2 * (c.x - a.x) * (b.y - a.y) - 2 * (b.x - a.x) * (c.y - a.y)),
			((b.x - a.x) * (c.x * c.x - a.x * a.x + c.y * c.y - a.y * a.y) - (c.x - a.x) * (b.x * b.x - a.x * a.x + b.y * b.y - a.y * a.y)) /
			(2 * (c.y - a.y) * (b.x - a.x) - 2 * (b.y - a.y) * (c.x - a.x)) };


		Circlef circle{};
		circle.radius = radius;
		circle.center = center;

		return circle;
	}
};


class Graph
{
public:
	Graph() = default;

	void SetPoints(const std::vector<Vertex>& pointsIn, std::vector<Vertex>& pointsOut)
	{
		m_PointList = pointsIn;
		CalculateSuperTriangle();
	}

	void CalculateTriangulation()
	{
		m_Triangulation.clear();

		m_Triangulation.push_back(m_SuperTriangle);

		for (const auto& point : m_PointList)
		{
			m_BadTriangles.clear();
			
			for (const auto& triangle : m_Triangulation)
			{
				if (utils::IsPointInCircle(Point2f{ point.x, point.y }, triangle.circumCircle))
					m_BadTriangles.push_back(triangle);
			}

			std::vector<Connection> polygon;
			for (const auto& badTriangle : m_BadTriangles) 
			{
				for (const auto& edge : badTriangle.edges) 
				{
					bool isShared = false;
					for (const auto& badTriangleToCheck : m_BadTriangles) 
					{
						if (badTriangle == badTriangleToCheck) continue;
						for (const auto& edgeToCheckAgainst : badTriangleToCheck.edges) 
						{
							if (edge == edgeToCheckAgainst) 
							{
								isShared = true;
								break;
							}
						}
						if (isShared) break;
					}
					if (!isShared) polygon.push_back(edge);
				}
			}

			//remove bad triangles from triangulation
			std::vector<Triangle>::iterator it = m_Triangulation.begin();
			while (it != m_Triangulation.end()) {
				if (std::find(m_BadTriangles.begin(), m_BadTriangles.end(), *it) != m_BadTriangles.end()) {
					it = m_Triangulation.erase(it);
				}
				else {
					++it;
				}
			}

			for (const auto& edge : polygon) {
				Triangle newTriangle{ edge.start, edge.end, point };
				m_Triangulation.push_back(newTriangle);
			}
		}

		//clean the lines up, remove the super triangle
		std::vector<Triangle> trianglesToRemove;
		for (const auto& triangle : m_Triangulation) {
			 if (triangle.a == m_SuperTriangle.a || triangle.a == m_SuperTriangle.b || triangle.a == m_SuperTriangle.c
			|| triangle.b == m_SuperTriangle.a || triangle.b == m_SuperTriangle.b || triangle.b == m_SuperTriangle.c
			|| triangle.c == m_SuperTriangle.a || triangle.c == m_SuperTriangle.b || triangle.c == m_SuperTriangle.c)
			 {
				trianglesToRemove.push_back(triangle);
			}
		}

		for (const auto& triangleToRemove : trianglesToRemove) {
			m_Triangulation.erase(std::remove(m_Triangulation.begin(), m_Triangulation.end(), triangleToRemove), m_Triangulation.end());
		}
	}
	void CalculateMST()
	{
		FillEdges();

		std::vector<Connection> mst;

		std::vector<Vertex> vertices{m_PointList};
		vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());

		int currentMSTGroup{ INVALID_MST_GROUP + 1 };
		for (auto& edge : m_Edges)
		{
			if (edge.start.MSTGroup == edge.end.MSTGroup && edge.start.MSTGroup != INVALID_MST_GROUP) continue;


			if (edge.end.MSTGroup == INVALID_MST_GROUP && edge.start.MSTGroup == INVALID_MST_GROUP)
			{
				edge.end.MSTGroup = currentMSTGroup;
				edge.start.MSTGroup = currentMSTGroup;
				++currentMSTGroup;

				for (auto& edgeToCheck : m_Edges)
				{
					if (edgeToCheck.end == edge.end) edgeToCheck.end.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.start == edge.end) edgeToCheck.start.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.end == edge.start) edgeToCheck.end.MSTGroup = edge.start.MSTGroup;
					if (edgeToCheck.start == edge.start) edgeToCheck.start.MSTGroup = edge.start.MSTGroup;
				}
			}

			else if (edge.start.MSTGroup != INVALID_MST_GROUP && edge.end.MSTGroup == INVALID_MST_GROUP)
			{
				edge.end.MSTGroup = edge.start.MSTGroup;
				++currentMSTGroup;

				for (auto& edgeToCheck : m_Edges)
				{
					if (edgeToCheck.end == edge.end) edgeToCheck.end.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.start == edge.end) edgeToCheck.start.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.end == edge.start) edgeToCheck.end.MSTGroup = edge.start.MSTGroup;
					if (edgeToCheck.start == edge.start) edgeToCheck.start.MSTGroup = edge.start.MSTGroup;
				}
			}

			else if (edge.end.MSTGroup != INVALID_MST_GROUP && edge.start.MSTGroup == INVALID_MST_GROUP)
			{
				edge.start.MSTGroup = edge.end.MSTGroup;
				++currentMSTGroup;

				for (auto& edgeToCheck : m_Edges)
				{
					if (edgeToCheck.end == edge.end) edgeToCheck.end.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.start == edge.end) edgeToCheck.start.MSTGroup = edge.end.MSTGroup;
					if (edgeToCheck.end == edge.start) edgeToCheck.end.MSTGroup = edge.start.MSTGroup;
					if (edgeToCheck.start == edge.start) edgeToCheck.start.MSTGroup = edge.start.MSTGroup;
				}
			}

			else if (edge.end.MSTGroup != INVALID_MST_GROUP && edge.start.MSTGroup != INVALID_MST_GROUP && edge.end.MSTGroup != edge.start.MSTGroup)
			{
				const int groupToStay{ edge.start.MSTGroup };
				const int groupToChange{ edge.end.MSTGroup };
				for (auto& edgeToChange : m_Edges)
				{
					if (edgeToChange.start.MSTGroup == groupToChange) edgeToChange.start.MSTGroup = groupToStay;
					if (edgeToChange.end.MSTGroup == groupToChange) edgeToChange.end.MSTGroup = groupToStay;
				}
			}

			mst.emplace_back(edge);
		}

		m_MSTEdges = mst;
	}

	void DebugDraw() const
	{
		//Draw Super Triangle
		Point2f a = Point2f{ m_SuperTriangle.a.x, m_SuperTriangle.a.y };
		Point2f b = Point2f{ m_SuperTriangle.b.x, m_SuperTriangle.b.y };
		Point2f c = Point2f{ m_SuperTriangle.c.x, m_SuperTriangle.c.y };
		utils::SetColor(Color4f{ 1,0,0,1 });
		utils::DrawPolygon({ a,b,c }, true, 10.f);

		//Draw Circum Circle of Super Triangle
		Ellipsef circum { m_SuperTriangle.circumCircle.center, m_SuperTriangle.circumCircle.radius, m_SuperTriangle.circumCircle.radius };
		utils::SetColor(colors::green);
		utils::FillEllipse(m_SuperTriangle.circumCircle.center, 5, 5);
		utils::DrawEllipse(circum);

		for (const auto& triangle : m_Triangulation)
		{
			utils::SetColor(colors::green);
			utils::DrawPolygon( {Point2f{ triangle.a.x, triangle.a.y }, Point2f{ triangle.b.x, triangle.b.y }, Point2f{ triangle.c.x, triangle.c.y }} );
		}

		for (const auto& point : m_PointList)
		{
			utils::DrawEllipse(Point2f{ point.x, point.y }, 5, 5);
		}

		for (const auto& edge : m_Edges)
		{
			const Texture edgeWeight("W:" + std::to_string(int(std::round(edge.weight))), "Fonts/dogica.ttf", 7, colors::white);
			edgeWeight.Draw(Point2f{ (edge.start.x + edge.end.x) / 2.f, (edge.start.y + edge.end.y) / 2.f });
		}

		for (const auto& edge : m_MSTEdges)
		{
			utils::SetColor(colors::blue);
			utils::DrawLine(edge.start.x, edge.start.y, edge.end.x, edge.end.y);
		}
	}

private:
	//Member Variables
	Triangle m_SuperTriangle{};
	std::vector<Triangle> m_Triangulation{};
	std::vector<Triangle> m_BadTriangles{};
	std::vector<Vertex> m_PointList{};
	std::vector<Connection> m_Edges{};
	std::vector<Connection> m_MSTEdges{};

	//Function Definitions
	void CalculateSuperTriangle()
	{
		//Predefine vertices of the SuperTriangle 
		Vertex a{}, b{}, c{};

		//Find the point furthest up that is made up from other triangle coordinates
		Vector2f topRight{ Vector2f{ m_PointList[0].x, m_PointList[0].y} };
		for (const auto& point : m_PointList)
		{
			if (point.x > topRight.x) topRight.x = point.x;
			if (point.y > topRight.y) topRight.y = point.y;
		}
		Vector2f bottomLeft{ Vector2f{ m_PointList[0].x, m_PointList[0].y} };
		for (const auto& point : m_PointList)
		{
			if (point.x < bottomLeft.x) bottomLeft.x = point.x;
			if (point.y < bottomLeft.y) bottomLeft.y = point.y;
		}

		//Calculate points based on the furthest points we found before
		const float rectSize{ std::abs(topRight.x - bottomLeft.x) };
		Vector2f pointA{ topRight.x - rectSize / 2.f, topRight.y * 2};
		Vector2f pointB{ topRight.x * 2, bottomLeft.y / 2};
		Vector2f pointC{ bottomLeft.x - 1.5f * rectSize, bottomLeft.y };

		const float bottomOffset{ rectSize / 5.f };
		a = { pointA.x, pointA.y };
		b = { pointB.x, pointB.y - bottomOffset };
		c = { pointC.x, pointC.y - bottomOffset };

		m_SuperTriangle = Triangle(a,b,c);
	}

	void FillEdges()
	{
		std::vector<Connection> edges{};

		//Add all of the edges that exist into an array
		for (const auto& triangle : m_Triangulation)
		{
			for (const auto& edge : triangle.edges)
			{
				edges.emplace_back(edge);
			}
		}

		//Remove all the duplicates while also sorting the array
		std::sort(edges.begin(), edges.end());
		edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

		m_Edges = edges;
	}
};