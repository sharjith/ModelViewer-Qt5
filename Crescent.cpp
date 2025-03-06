﻿#include "Crescent.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

Crescent::Crescent(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Crescent");
	buildMesh();
}

Crescent::~Crescent()
{
}

TriangleMesh* Crescent::clone()
{
	return new Crescent(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float Crescent::firstUParameter() const
{
	return 0.0;
}

float Crescent::lastUParameter() const
{
	return 1.0;
}

float Crescent::firstVParameter() const
{
	return 0.0;
}

float Crescent::lastVParameter() const
{
	return 1.0;
}

Point Crescent::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/
	/*Crescent
	Where 0 <= u <= 1, 0 <= v <= 1
	*/
	x = _radius * ((2 + sin(2 * glm::pi<float>() * u) * sin(2 * glm::pi<float>() * v)) * cos(3 * glm::pi<float>() * v));
	y = _radius * ((2 + sin(2 * glm::pi<float>() * u) * sin(2 * glm::pi<float>() * v)) * sin(3 * glm::pi<float>() * v));
	z = _radius * (cos(2 * glm::pi<float>() * u) * sin(2 * glm::pi<float>() * v) + 4 * v - 2);

	P.setParam(x, y, z);
	return P;
}