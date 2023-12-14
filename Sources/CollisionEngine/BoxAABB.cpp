#include "BoxAABB.h"
#include <iostream>

#pragma region Buffer

void CBoxAABB::CreateBuffers()
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

void CBoxAABB::BindBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (void*)0);
	}
}

void CBoxAABB::DestroyBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glDeleteBuffers(1, &m_vertexBufferId);
		m_vertexBufferId = 0;
	}
}

#pragma endregion

void CBoxAABB::RefreshMinAndMax(std::vector<Vec2>& myPoints)
{
	minPoint = myPoints.front();
	maxPoint = myPoints.front();

	for (Vec2 point : myPoints)
	{
		// test Max
		if (point.x > maxPoint.x)
			maxPoint.x = point.x;
		if (point.y > maxPoint.y)
			maxPoint.y = point.y;

		// test Min
		if (point.x < minPoint.x)
			minPoint.x = point.x;
		if (point.y < minPoint.y)
			minPoint.y = point.y;
	}
}

void CBoxAABB::Build(std::vector<Vec2>& myPoints, const Vec2& pos)
{
	position = pos;

	RefreshMinAndMax(myPoints);

	points.clear();

	points.push_back(minPoint);
	points.push_back({ maxPoint.x, minPoint.y });
	points.push_back(maxPoint);
	points.push_back({ minPoint.x, maxPoint.y });

	CreateBuffers();
}

void CBoxAABB::Draw(Vec2 position)
{// Set transforms (qssuming model view mode is set)
	float transfMat[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f,
							position.x, position.y, -1.0f, 1.0f };
	glPushMatrix();
	glMultMatrixf(transfMat);

	BindBuffers();
	glDrawArrays(GL_LINE_LOOP, 0, points.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
}

bool CBoxAABB::isColliding(CBoxAABB& other,const Vec2& otherPos)
{

	return (minPoint.x + position.x < other.maxPoint.x + otherPos.x &&
			maxPoint.x + position.x > other.minPoint.x + otherPos.x &&
			minPoint.y + position.y < other.maxPoint.y + otherPos.y &&
			maxPoint.y + position.y > other.minPoint.y + otherPos.y );
}
