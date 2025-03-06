﻿#include "Spring.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

Spring::Spring(QOpenGLShaderProgram* prog, float sectionRadius, float coilRadius, float pitch, float turns, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_sectionRadius(sectionRadius),
	_coilRadius(coilRadius),
	_pitch(pitch),
	_turns(turns)
{
	setAutoIncrName("Spring");
	buildMesh();
}

Spring::~Spring()
{
}

TriangleMesh* Spring::clone()
{
	return new Spring(_prog, _sectionRadius, _coilRadius, _pitch, _turns, _slices, _stacks, _sMax, _tMax);
}

float Spring::firstUParameter() const
{
	return 0.0;
}

float Spring::lastUParameter() const
{
	return _turns * glm::two_pi<float>();
}

float Spring::firstVParameter() const
{
	return 0.0;
}

float Spring::lastVParameter() const
{
	return glm::two_pi<float>();
}

Point Spring::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/spiral/
	// Spring

	float h = (1 / glm::pi<float>()) / _sectionRadius * _pitch;

	x = (_coilRadius + _sectionRadius * cos(v)) * cos(u);
	y = (_coilRadius + _sectionRadius * cos(v)) * sin(u);
	z = _sectionRadius * (sin(v) + u * h);

	P.setParam(x, y, z);
	return P;
}