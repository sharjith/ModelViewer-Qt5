﻿#include "SteinerSurface.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

SteinerSurface::SteinerSurface(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Steiner Surface");
	buildMesh();
}

SteinerSurface::~SteinerSurface()
{
}

TriangleMesh* SteinerSurface::clone()
{
	return new SteinerSurface(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float SteinerSurface::firstUParameter() const
{
	return 0.0;
}

float SteinerSurface::lastUParameter() const
{
	return glm::pi<float>();
}

float SteinerSurface::firstVParameter() const
{
	return 0.0;
}

float SteinerSurface::lastVParameter() const
{
	return glm::pi<float>();
}

Point SteinerSurface::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	// Steiner surface
	// Where 0 <= u <= pi, 0 <= v <= pi
	x = _radius * cos(v) * cos(v) * sin(2 * u) / 2;
	y = _radius * sin(u) * sin(2 * v) / 2;
	z = _radius * cos(u) * sin(2 * v) / 2;

	P.setParam(x, y, z);
	return P;
}