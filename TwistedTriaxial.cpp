﻿#include "TwistedTriaxial.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

TwistedTriaxial::TwistedTriaxial(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Twisted Triaxial");
	buildMesh();
}

TwistedTriaxial::~TwistedTriaxial()
{
}

TriangleMesh* TwistedTriaxial::clone()
{
	return new TwistedTriaxial(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float TwistedTriaxial::firstUParameter() const
{
	return -glm::pi<float>();
}

float TwistedTriaxial::lastUParameter() const
{
	return glm::pi<float>();
}

float TwistedTriaxial::firstVParameter() const
{
	return -glm::pi<float>();
}

float TwistedTriaxial::lastVParameter() const
{
	return glm::pi<float>();
}

Point TwistedTriaxial::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/

	// Twisted Triaxial
	// Where -pi <= u <= pi, -pi <= v <= pi
	float PI = glm::pi<float>();
	float pp = sqrt(u * u + v * v) / sqrt(2 * PI * PI);
	float TWOPI = glm::two_pi<float>();
	x = _radius * (1 - pp) * cos(u) * cos(v) + pp * sin(u) * sin(v);
	y = _radius * (1 - pp) * cos(u + TWOPI / 3) * cos(v + TWOPI / 3) + pp * sin(u + TWOPI / 3) * sin(v + TWOPI / 3);
	z = _radius * (1 - pp) * cos(u + 4 * PI / 3) * cos(v + 4 * PI / 3) + pp * sin(u + 4 * PI / 3) * sin(v + 4 * PI / 3) - _radius / 5;

	P.setParam(x, y, z);
	return P;
}