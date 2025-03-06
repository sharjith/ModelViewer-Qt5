﻿#include "Periwinkle.h"
#include "Point.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

Periwinkle::Periwinkle(QOpenGLShaderProgram* prog, float radius, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) :
	ParametricSurface(prog, nSlices, nStacks, sMax, tMax),
	_radius(radius)
{
	setAutoIncrName("Periwinkle Sea Shell");
	buildMesh();
}

Periwinkle::~Periwinkle()
{
}

TriangleMesh* Periwinkle::clone()
{
	return new Periwinkle(_prog, _radius, _slices, _stacks, _sMax, _tMax);
}

float Periwinkle::firstUParameter() const
{
	return 0.0;
}

float Periwinkle::lastUParameter() const
{
	return glm::two_pi<float>();
}

float Periwinkle::firstVParameter() const
{
	return 0.0;
}

float Periwinkle::lastVParameter() const
{
	return glm::two_pi<float>();
}

Point Periwinkle::pointAtParameter(const float& u, const float& v)
{
	Point P;
	float x, y, z;

	// http://xahlee.info/SpecialPlaneCurves_dir/Seashell_dir/seashell_math_formulas.html

	// Periwinkle
	float R = 1;    // radius of tube
	float N = 4.6f;  // number of turns
	float H = 2.0f; // height
	float p = 2;    // power
	auto W = [R](auto u) {return u / (2 * glm::pi<float>()) * R; };
	x = _radius * (W(u) * cos(N * u) * (1 + cos(v)));
	y = _radius * (W(u) * sin(N * u) * (1 + cos(v)));
	z = _radius * (W(u) * sin(v) + H * pow((u / (2 * glm::pi<float>())), p)) - _radius * 1.5;

	P.setParam(x, y, z);
	return P;
}