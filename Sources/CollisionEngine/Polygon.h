#ifndef _POLYGON_H_
#define _POLYGON_H_

#include <GL/glew.h>
#include <memory>
#include <array>
#include <algorithm>

#include "BoxAABB.h"

#pragma region SimplexStruct

struct SCollision;

struct Simplex {

private:
	unsigned m_size = 0;

public:
	std::array<Vec2, 3> m_points;

	Simplex()
	{}

	Simplex& operator=(std::initializer_list<Vec2> list) {
		for (auto v = list.begin(); v != list.end(); v++) {
			m_points[std::distance(list.begin(), v)] = *v;
		}
		m_size = list.size();

		return *this;
	}

	void push_front(Vec2 point) {
		m_points = { point, m_points[0], m_points[1] };
		m_size = m_size <= 3 ? m_size + 1 : 3;
	}

	void remove(int element) 
	{
		//auto it = std::find(m_points.begin(), m_points.end(), element);
		//if (it != m_points.end()) {
		//	m_points.(it);
		//}
		
	}

	Vec2& operator[](unsigned i) { return m_points[i]; }
	unsigned size() const { return m_size; }
};

#pragma endregion

class CPolygon
{
private:
	friend class CWorld;

	CPolygon(size_t index);
public:
	~CPolygon();

	Vec2				position;
	Mat2				rotation;
	std::vector<Vec2>	points;

	void				Build();
	void				Draw();
	void				DrawAABB();
	size_t				GetIndex() const;

	Vec2				GetWolrdMinAABB();
	Vec2				GetWolrdMaxAABB();


	Vec2				TransformPoint(const Vec2& point) const;
	Vec2				InverseTransformPoint(const Vec2& point) const;
	Vec2				FindFurthestPoint(Vec2 direction) const;

	void				ComputeArea();

	float				GetArea() const;

	void				RecenterOnCenterOfMass();

	float				GetMass() const;
	float				GetInertiaTensor() const;

	void				ComputeLocalInertiaTensor();

	// if point is outside then returned distance is negative (and doesn't make sense)
	bool				IsPointInside(const Vec2& point) const;

	bool				CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const;
	bool				IsMovingPositionAndRotation();

	float				GetDistanceAndNormal(Simplex& simplexPoints, Vec2& norm) const;
	void				GetInfoCollisionWithEPA(Simplex& simplexPoints,const CPolygon& shapeA, const CPolygon& shapeB, Vec2& colNormal, float& colDistance) const;
	SCollision			EPA(const Simplex& simplex, const CPolygon& poly) const;
	// Physics
	float				density;

	Vec2				speed;

	float				angularVelocity = 0.0f;
	Vec2				forces;
	float				torques = 0.0f;

	CBoxAABB			boxAABB;

	bool				isCollide = false;


private:
	void				CreateBuffers();
	void				BindBuffers();
	void				DestroyBuffers();

	void				BuildLines();

	bool				CheckSimplexTriangle(Simplex& simplexPoints, Vec2& direction) const;

	GLuint				m_vertexBufferId;
	size_t				m_index;

	std::vector<Line>	m_lines;

	Vec2				savePosition;
	Mat2				saveRotation;

	float				m_signedArea;
	float				m_localInertiaTensor;
};

typedef std::shared_ptr<CPolygon>	CPolygonPtr;

#endif