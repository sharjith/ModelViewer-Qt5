﻿#include "SuperToroid.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

SuperToroid::SuperToroid(QOpenGLShaderProgram* prog, float outerRadius, float innerRadius, float n1, float n2, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_outerRadius(outerRadius),
	_innerRadius(innerRadius),
	_n1(n1),
	_n2(n2)
{
	setAutoIncrName("Super Toroid");
	buildMesh();
}

SuperToroid::~SuperToroid()
{
}

TriangleMesh* SuperToroid::clone()
{
	return new SuperToroid(_prog, _outerRadius, _innerRadius, _n1, _n2, _slices, _stacks, _sMax, _tMax);
}

float SuperToroid::firstUParameter() const
{
	return 0.0;
}

float SuperToroid::lastUParameter() const
{
	return glm::two_pi<float>();
}

float SuperToroid::firstVParameter() const
{
	return 0.0;
}

float SuperToroid::lastVParameter() const
{
	return glm::two_pi<float>();
}

Point SuperToroid::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	//http://paulbourke.net/geometry/toroidal/
	// Super Toroid
	// x = cos^n1(u) * (r0 + r1 * cos^n2(v))
	// y = sin^n1(u) * (r0 + r1 * cos^n2(v))
	// z = rl * sin^n2(v)
	// Where u = 0 - 2PI and v = 0 - 2PI

	auto power = [](double f, double p)->double
	{
		int sign;
		double absf;

		sign = (f < 0 ? -1 : 1);
		absf = (f < 0 ? -f : f);

		if (absf < 0.00001)
			return(0.0);
		else
			return(sign * pow(absf, p));
	};

	x = power(cos(u), _n1) * (_outerRadius + _innerRadius * power(cos(v), _n2));
	y = power(sin(u), _n1) * (_outerRadius + _innerRadius * power(cos(v), _n2));
	z = _innerRadius * power(sin(v), _n2);

	P.setParam(x, y, z);
	return P;
}