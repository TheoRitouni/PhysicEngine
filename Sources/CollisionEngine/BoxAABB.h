#pragma once

#include <vector>
#include <GL/glew.h>

#include "Maths.h"

class CBoxAABB 
{
private:

	GLuint m_vertexBufferId;

public:

	Vec2 minPoint, maxPoint;
	Vec2 position;

	bool isCollide = false;

	std::vector<Vec2> points;

	void Build(std::vector<Vec2>& points, const Vec2& pos);
	void RefreshMinAndMax(std::vector<Vec2>& points);

	void CreateBuffers();
	void BindBuffers();
	void DestroyBuffers();

	void Draw(Vec2 position);
	bool isColliding(CBoxAABB& other, const Vec2& otherPos);
};

