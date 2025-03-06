﻿#include "KleinBottle.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

KleinBottle::KleinBottle(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Klein Bottle");
	buildMesh();
}

KleinBottle::~KleinBottle()
{
}

TriangleMesh* KleinBottle::clone()
{
	return new KleinBottle(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float KleinBottle::firstUParameter() const
{
	return 0.0;
}

float KleinBottle::lastUParameter() const
{
	return glm::two_pi<float>();
}

float KleinBottle::firstVParameter() const
{
	return 0.0;
}

float KleinBottle::lastVParameter() const
{
	return glm::two_pi<float>();
}

Point KleinBottle::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/
	// Klein Bottle
	// Where u = 0 - 2PI and v = 0 - 2PI
	float r = 4 * (1 - cos(u) / 2);
	if (u >= 0 && u < glm::pi<float>())
		x = -_radius / 6 * (6 * cos(u) * (1 + sin(u)) + r * cos(u) * cos(v));
	else
		x = -_radius / 6 * (6 * cos(u) * (1 + sin(u)) + r * cos(v + glm::pi<float>()));

	if (u >= 0 && u < glm::pi<float>())
		z = -_radius / 6 * (16 * sin(u) + r * sin(u) * cos(v));
	else
		z = -_radius / 6 * (16 * sin(u));

	y = -_radius / 6 * (r * sin(v));

	P.setParam(x, y, z);
	return P;
}