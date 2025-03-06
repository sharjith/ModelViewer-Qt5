﻿#include "DoubleCone.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

DoubleCone::DoubleCone(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Double Cone");
	buildMesh();
}

DoubleCone::~DoubleCone()
{
}

TriangleMesh* DoubleCone::clone()
{
	return new DoubleCone(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float DoubleCone::firstUParameter() const
{
	return 0.0;
}

float DoubleCone::lastUParameter() const
{
	return glm::two_pi<float>();
}

float DoubleCone::firstVParameter() const
{
	return -1.0;
}

float DoubleCone::lastVParameter() const
{
	return 1.0;
}

Point DoubleCone::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/
	// Double Cone
	// where 0 < u < 2pi, -1 < v < 1
	x = _radius * v * cos(u);
	y = _radius * (v - 1) * cos(u + 2 * glm::pi<float>() / 3);
	z = _radius * (1 - v) * cos(u - 2 * glm::pi<float>() / 3);

	P.setParam(x, y, z);
	return P;
}