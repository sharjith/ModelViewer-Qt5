﻿#include "BreatherSurface.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

BreatherSurface::BreatherSurface(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Breather Surface");
	buildMesh();
}

BreatherSurface::~BreatherSurface()
{
}

TriangleMesh* BreatherSurface::clone()
{
	return new BreatherSurface(_prog, _radius, _slices, _stacks, _sMax, _tMax);;
}

float BreatherSurface::firstUParameter() const
{
	return -13.2f;
}

float BreatherSurface::lastUParameter() const
{
	return 13.2f;
}

float BreatherSurface::firstVParameter() const
{
	return -37.4f;
}

float BreatherSurface::lastVParameter() const
{
	return 37.4f;
}

Point BreatherSurface::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://xahlee.info/surface/breather_p/breather_p.html

	float b = 0.4f;
	float r = 1 - b * b;
	float w = sqrt(r);
	float denom = b * pow((w * cosh(b * u)), 2) + pow((b * sin(w * v)), 2);
	x = _radius * (-u + (2 * r * cosh(b * u) * sinh(b * u)) / denom);
	y = _radius * ((2 * w * cosh(b * u) * (-(w * cos(v) * cos(w * v)) - sin(v) * sin(w * v))) / denom);
	z = _radius * ((2 * w * cosh(b * u) * (-(w * sin(v) * cos(w * v)) + cos(v) * sin(w * v))) / denom);

	P.setParam(x, y, z);
	return P;
}