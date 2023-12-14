#include "Polygon.h"
#include <GL/glu.h>
#include <iostream>
#include <string>

#include "GlobalVariables.h"
#include "Renderer.h" 

#include "PhysicEngine.h"

CPolygon::CPolygon(size_t index)
	: m_vertexBufferId(0), m_index(index), density(0.1f)
{
}

CPolygon::~CPolygon()
{
	boxAABB.DestroyBuffers();
	DestroyBuffers();
}

void CPolygon::Build()
{
	m_lines.clear();
	
	ComputeArea();
	RecenterOnCenterOfMass();
	ComputeLocalInertiaTensor();

	CreateBuffers();
	BuildLines();
}

void CPolygon::Draw()
{
	if (isCollide)
		glColor3f(0.0f, 1.0f, 0.0f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);

	// Set transforms (qssuming model view mode is set)
	float transfMat[16] = {	rotation.X.x, rotation.X.y, 0.0f, 0.0f,
							rotation.Y.x, rotation.Y.y, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f,
							position.x, position.y, -1.0f, 1.0f };
	glPushMatrix();
	glMultMatrixf(transfMat);

	// Draw vertices
	BindBuffers();
	glDrawArrays(GL_LINE_LOOP, 0, points.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	glColor3f(0.0f,0.0f,0.0f);

}

size_t	CPolygon::GetIndex() const
{
	return m_index;
}

Vec2	CPolygon::TransformPoint(const Vec2& point) const
{
	return position + rotation * point;
}

Vec2	CPolygon::InverseTransformPoint(const Vec2& point) const
{
	return rotation.GetInverse() * (point - position);
}

bool	CPolygon::IsPointInside(const Vec2& point) const
{
	float maxDist = -FLT_MAX;

	for (const Line& line : m_lines)
	{
		Line globalLine = line.Transform(rotation, position);
		float pointDist = globalLine.GetPointDist(point);
		maxDist = Max(maxDist, pointDist);
	}

	return maxDist <= 0.0f;
}

void CPolygon::CreateBuffers()
{
	DestroyBuffers();

	float* vertices = new float[3 * points.size()];
	for (size_t i = 0; i < points.size(); ++i)
	{
		vertices[3 * i] = points[i].x;
		vertices[3 * i + 1] = points[i].y;
		vertices[3 * i + 2] = 0.0f;
	}

	glGenBuffers(1, &m_vertexBufferId);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * points.size(), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] vertices;
}

void CPolygon::BindBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (void*)0);
	}
}

void CPolygon::DestroyBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glDeleteBuffers(1, &m_vertexBufferId);
		m_vertexBufferId = 0;
	}
}

void CPolygon::BuildLines()
{
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];

		Vec2 lineDir = (pointA - pointB).Normalized();

		m_lines.push_back(Line(pointB, lineDir));
	}
}

bool CPolygon::IsMovingPositionAndRotation()
{
	return ((savePosition.x != position.x && savePosition.y != position.y) ||
		   ((saveRotation.X.x != rotation.X.x && saveRotation.X.y != rotation.X.y) &&
			(saveRotation.Y.x != rotation.Y.x && saveRotation.Y.y != rotation.Y.y)));
}

void CPolygon::DrawAABB()
{
	if(boxAABB.isCollide)
		glColor3f(0.0f, 1.0f, 0.0f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);

	if (IsMovingPositionAndRotation())
	{
		std::vector<Vec2> newPoints;

		for (Vec2 point : points)
			newPoints.push_back(rotation * point);	

		boxAABB.Build(newPoints,position);
	}

	boxAABB.Draw(position);

	savePosition = position;
	saveRotation = rotation;

	glColor3f(0.0f, 0.0f, 0.0f);

}

Vec2 CPolygon::GetWolrdMinAABB()
{
	return boxAABB.minPoint + position;
}
Vec2 CPolygon::GetWolrdMaxAABB()
{
	return boxAABB.maxPoint + position;
}




/*

	NARROW PHASE

*/

#pragma region NarrowPhase

Vec2 CPolygon::FindFurthestPoint(Vec2 direction) const
{
	Vec2 maxPoint;
	float maxDistance = -FLT_MAX;

	for (Vec2 vertex : points)
	{
		vertex = TransformPoint(vertex);

		float distance = vertex | direction;
		

		if (distance > maxDistance)
		{
			maxDistance = distance;
			maxPoint = vertex;
		}
	}

	return maxPoint;
}

void CPolygon::ComputeArea()
{
	m_signedArea = 0.0f;
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];
		m_signedArea += pointA.x * pointB.y - pointB.x * pointA.y;
	}
	m_signedArea *= 0.5f;
}

float	CPolygon::GetArea() const
{
	return fabsf(m_signedArea);
}

void CPolygon::RecenterOnCenterOfMass()
{
	Vec2 centroid;
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];
		float factor = pointA.x * pointB.y - pointB.x * pointA.y;
		centroid.x += (pointA.x + pointB.x) * factor;
		centroid.y += (pointA.y + pointB.y) * factor;
	}
	centroid /= 6.0f * m_signedArea;

	for (Vec2& point : points)
	{
		point -= centroid;
	}
	position += centroid;
}

float CPolygon::GetMass() const
{
	return density * GetArea();
}

float CPolygon::GetInertiaTensor() const
{
	if (GetMass() == 0.0f)
		return m_localInertiaTensor * 0.1f;
	else
		return m_localInertiaTensor * GetMass();
}

float ComputeInertiaTensor_BaseHalfbaseHeight(float b, float a, float h)
{
	float b2 = b * b;
	float b3 = b2 * b;
	float a2 = a * a;
	float a3 = a2 * a;
	float h3 = h * h * h;

	return (b3 * h - b2 * h * a + b * h * a2 + b * h3) * (1.0f / 36.0f);
}

// consider A is the center of gravity
float	ComputeInertiaTensor_Triangle(const Vec2& A, const Vec2& B, const Vec2& C)
{
	Vec2 AB = B - A;
	Vec2 AC = C - A;

	Vec2 G = (A + B + C) / 3.0f;

	float base, halfBase, height;
	if (AB.GetSqrLength() >= AC.GetSqrLength())
	{
		base = AB.GetLength();
		halfBase = AC | (AB / base);
		height = (AC - AB * (halfBase / base)).GetLength();
	}
	else
	{
		base = AC.GetLength();
		halfBase = AB | (AC / base);
		height = (AB - AC * (halfBase / base)).GetLength();
	}


	return ComputeInertiaTensor_BaseHalfbaseHeight(base, halfBase, height) + (G - A).GetSqrLength();
}


void CPolygon::ComputeLocalInertiaTensor()
{
	m_localInertiaTensor = 0.0f;
	for (size_t i = 0; i + 1 < points.size(); ++i)
	{
		const Vec2& pointA = points[i];
		const Vec2& pointB = points[i + 1];

		m_localInertiaTensor += ComputeInertiaTensor_Triangle(Vec2(), pointA, pointB);
	}
}



Vec2 tripleProduct(Vec2 a, Vec2 b, Vec2 c) 
{

	Vec2 vec;

	float ac = a | c;
	float bc = b | c;

	vec.x = b.x * ac - a.x * bc;
	vec.y = b.y * ac - a.y * bc;

	return vec;
}

 bool LineFct(Simplex points, Vec2& direction)
{
	Vec2 a = points[0];
	Vec2 b = points[1];

	Vec2 ab = b - a;
	Vec2 ao = -a;

	Vec2 ABperp = tripleProduct(ab, ao, ab);
	direction = ABperp;
	return false;
}

bool CPolygon::CheckSimplexTriangle(Simplex& simplexPoints, Vec2& direction) const
{
	Vec2 a = simplexPoints[0];
	Vec2 b = simplexPoints[1];
	Vec2 c = simplexPoints[2];

	Vec2 ab = b - a;
	Vec2 ac = c - a;
	Vec2 ao = -a;

	Vec2 abProd = tripleProduct(ac, ab, ab);
	Vec2 acProd = tripleProduct(ab, ac, ac);
	
	if ((abProd | ao) > 0)
	{	
		return LineFct(simplexPoints = { a,b }, direction);
	}

	if ((acProd | ao) > 0)
	{
		return LineFct(simplexPoints = { a,c }, direction);
	}

	return true;
}


Vec2 GetPointGJK( const CPolygon& polyA, const CPolygon& polyB, Vec2 direction)
{
	return polyA.FindFurthestPoint(direction) - polyB.FindFurthestPoint(-direction);
}

Vec2 GetProjectionPoint(Vec2 startPoint, Vec2 line, Vec2 point)
{
	double val = line | point;
	double len2 = line.GetSqrLength();

	Vec2 newPoint = Vec2((startPoint.x + (val * line.x) / len2), (startPoint.y + (val * line.y) / len2));

	Vec2 max = Vec2(FLT_MAX, FLT_MAX);

	double lineDotPoint = line | (newPoint - startPoint);
	if (lineDotPoint < 0) return max;
	if (lineDotPoint == 0) return max;

	// Compute the square of the segment length 
	double lineDot = line | line;
	if (lineDotPoint > lineDot) return max;
	if (lineDotPoint == lineDot) return max;

	return newPoint;
}

float CPolygon::GetDistanceAndNormal(Simplex& simplexPoints, Vec2& norm) const
{
	float distance = FLT_MAX;

	Vec2 a = simplexPoints[0];
	Vec2 b = simplexPoints[1];
	Vec2 c = simplexPoints[2];

	if (a == b || b == c || a == c)
		return 0;

	std::array<Vec2, 3> pointsProj;

	pointsProj[0] = GetProjectionPoint(a, b - a, -a);
	pointsProj[1] = GetProjectionPoint(b, c - b, -b);
	pointsProj[2] = GetProjectionPoint(c, a - c, -c);

	Vec2 minPoint;

	for (Vec2 p : pointsProj)
	{
		if (p.GetLength() < distance )
		{
			distance = p.GetLength();
			norm = p.Normalized();
			minPoint = position + norm;
		}
	}

	if (gVars->bDebug)
	{
		gVars->pRenderer->DrawLine(Vec2(0, 0), norm * 5, 0.0, 2.0, 0.0);
		gVars->pRenderer->DisplayTextWorld("distance : " + std::to_string(distance), minPoint);
	}

	return distance ;

}

bool	CPolygon::CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const
{
	// narrow phase 
	 
	Vec2 direction = Vec2(1, 0);

	Simplex simp;

	simp.push_front(GetPointGJK(*this, poly, direction));

	direction = -simp[0];

	Vec2 newSimplexPoint = GetPointGJK(*this, poly, direction);
	
	if ((newSimplexPoint.Normalized() | direction.Normalized()) <= 0)
		return false;

	simp.push_front(newSimplexPoint);

	// Line
	Vec2 a = simp[0];
	Vec2 b = simp[1];

	Vec2 ab = b - a;
	Vec2 ao = -a;

	Vec2 abProd = tripleProduct(ab, ao, ab);
	direction = abProd;

	// you want more precision, higher iteration but less performancee
	int maxIter = 25;

	for (int i = 0; i < maxIter; i++)
	{
		newSimplexPoint = GetPointGJK(*this, poly, direction);

		if ((newSimplexPoint.Normalized() | direction.Normalized()) <= 0)
			return false;

		simp.push_front(newSimplexPoint);


		if (CheckSimplexTriangle(simp, direction))
		{
			SCollision coliderInfo = EPA(simp, poly);

			colDist = coliderInfo.distance;
			colNormal = coliderInfo.normal;
			colPoint = coliderInfo.point;

			if (gVars->bDebug)
			{
				gVars->pRenderer->DrawLine(this->position, colNormal * 5.f, 2, 0, 0);
				gVars->pRenderer->DrawLine(this->position, colPoint, 2, 2, 0);
			}


			return true;
		}
	}

	
	return false;
}

#pragma endregion


// my old epa doesn't work well 
void CPolygon::GetInfoCollisionWithEPA(Simplex& simp, const CPolygon& shapeA, const CPolygon& shapeB, Vec2& colNormal, float& colDistance) const
{
	int minIndex = 0;
	float minDistance = FLT_MAX;
	Vec2 minNormal;

	while (minDistance == FLT_MAX)
	{
		for (int i = 0; i < simp.size() - 1; i++)
		{
			int j = (i + 1) % 3;

			Vec2 vertexI = simp[i];
			Vec2 vertexJ = simp[j];

			Vec2 ij = vertexJ - vertexI;

			Vec2 normal = Vec2(ij.y, -ij.x).Normalized();
			float distance = normal | vertexI;

			if (distance < 0) {
				distance *= -1;
				normal = normal * (-1.0f);
			}

			if (distance < minDistance) {
				minDistance = distance;
				minNormal = normal;
				minIndex = j;
			}
		}

		Vec2 support = GetPointGJK(shapeA, shapeB, minNormal);
		float sDistance = minNormal | (support);

		if (abs(sDistance - minDistance) > 0.001) {
			minDistance = FLT_MAX;
			simp.push_front(support);
		}
	}

	colNormal = minNormal;
	colDistance = minDistance;

	//return colNormal * (colDistance + 0.001f);


}

SCollision  CPolygon::EPA(const Simplex& simplex, const CPolygon& poly) const
{
	int minInd = 0;
	Vec2 minNormal;
	float minDist = FLT_MAX;
	std::vector<Vec2> polytope(simplex.m_points.begin(), simplex.m_points.end());

	int maxIter = 32;
	for(int i = 0; i < maxIter; i++ )
	{
		for (int i = 0; i < polytope.size(); i++)
		{
			Vec2 a = polytope[i];
			Vec2 b = polytope[(i + 1) % polytope.size()];

			Vec2 ab = b - a;

			Vec2 normal = (Vec2(ab.y, -ab.x)).Normalized();

			float distance = normal | a;

			if (distance < 0)
			{
				normal *= -1;
				distance *= -1;
			}

			if (distance < minDist)
			{
				minNormal = normal;
				minDist = distance;
				minInd = i;
			}
		}

		Vec2 support = GetPointGJK(*this, poly, minNormal);
		float Distance = minNormal | support;
		float DeltaDist = Distance - minDist;
		float AbsDist = DeltaDist < 0 ? DeltaDist * -1 : DeltaDist;

		if (AbsDist > 0.001f)
		{
			minDist = FLT_MAX;
			polytope.insert(polytope.begin() + minInd + 1, support);
		}

		if (minDist != FLT_MAX)
			break;
	}
	if (minDist == FLT_MAX)

		return SCollision{};


	SCollision colision;
	colision.normal = minNormal;
	colision.distance = minDist;

	Vec2 point = FindFurthestPoint(minNormal) - (minNormal * minDist * 0.9999);
	if (!poly.IsPointInside(point))
		point = poly.FindFurthestPoint(-minNormal);

	colision.point = point;
	return colision;
}