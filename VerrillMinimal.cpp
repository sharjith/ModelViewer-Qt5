﻿#include "VerrillMinimal.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

VerrillMinimal::VerrillMinimal(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Verrill Minimal Surface");
	buildMesh();
}

VerrillMinimal::~VerrillMinimal()
{
}

TriangleMesh* VerrillMinimal::clone()
{
	return new VerrillMinimal(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float VerrillMinimal::firstUParameter() const
{
	return 0.0;
}

float VerrillMinimal::lastUParameter() const
{
	return glm::two_pi<float>();
}

float VerrillMinimal::firstVParameter() const
{
	return 0.5f;
}

float VerrillMinimal::lastVParameter() const
{
	return 1.0f;
}

Point VerrillMinimal::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/
	//Verrill minimal surface
	// Where 0 <= u <= 2 pi and 0.5 <= v <= 1
	x = _radius * (-2 * v * cos(u) + (2 * cos(u)) / v - (2 * v * v * v * cos(3 * u)) / 3);
	y = _radius * (6 * v * sin(u) - (2 * sin(u)) / v - (2 * v * v * v * sin(3 * u)) / 3);
	z = _radius * (4 * log(v)) + _radius * 1.5;

	P.setParam(x, y, z);
	return P;
}