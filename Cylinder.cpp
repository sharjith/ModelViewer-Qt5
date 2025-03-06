#include "Cylinder.h"
#include "Point.h"

#include <cstdio>
#include <cmath>

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

Cylinder::Cylinder(QOpenGLShaderProgram* prog, float radius, float height, unsigned int nSlices, unsigned int nStacks, unsigned int sMax, unsigned int tMax) : GridMesh(prog, "Cylinder", nSlices, nStacks),
_radius(radius),
_height(height)
{
	_sMax = sMax;
	_tMax = tMax;
	int nVerts = ((nSlices + 1) * (nStacks + 1)) + (nSlices * 2) + 4;
	int elements = ((nSlices * 2 * (nStacks)) * 3) + (nSlices * 2) * 3;

	// Verts
	std::vector<float> p(3 * nVerts);
	// Normals
	std::vector<float> n(3 * nVerts);
	// Tangents
	std::vector<float> tg(3 * nVerts);
	// Bitangents
	std::vector<float> bt(3 * nVerts);
	// Tex coords
	std::vector<float> tex(2 * nVerts);
	// Elements
	std::vector<unsigned int> el(elements);

	// Generate positions and normals
	float theta, phi;
	float thetaFac = glm::two_pi<float>() / nSlices;
	float phiFac = 1.0f / nStacks;
	float nx, ny, nz, s, t;
	unsigned int idx = 0, tIdx = 0;
	for (unsigned int i = 0; i <= nSlices; i++)
	{
		theta = i * thetaFac;
		s = (float)i / nSlices * _sMax;
		for (unsigned int j = 0; j <= nStacks; j++)
		{
			phi = j * phiFac;
			t = (float)j / nStacks * _tMax;
			nx = cosf(theta);
			ny = sinf(theta);
			nz = (phi);
			glm::vec3 o(0, 0, (nz * height) - height / 2.0f);
			glm::vec3 v((nx * radius), (ny * radius), (nz * height) - height / 2.0f);
			p[idx + 0] = v.x;
			p[idx + 1] = v.y;
			p[idx + 2] = v.z;
			glm::vec3 normal = v - o;
			normal = glm::normalize(normal);
			n[idx + 0] = normal.x;
			n[idx + 1] = normal.y;
			n[idx + 2] = normal.z;
			tg[idx + 0] = 0.0f;
			tg[idx + 1] = 0.0f;
			tg[idx + 2] = 1.0f;
			glm::vec3 tangent = glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f));
			bt[idx + 0] = tangent.x;
			bt[idx + 1] = tangent.y;
			bt[idx + 2] = tangent.z;
			idx += 3;

			tex[tIdx] = s;
			tex[tIdx + 1] = t;
			tIdx += 2;
		}
	}

	// bottom face
	for (unsigned int i = 0; i <= nSlices; i++)
	{
		theta = i * thetaFac;		
		nx = cosf(theta);
		ny = sinf(theta);
		nz = 0;

		p[idx + 0] = radius * nx;
		p[idx + 1] = radius * ny;
		p[idx + 2] = nz - height / 2.0f;
		n[idx + 0] = 0;
		n[idx + 1] = 0;
		n[idx + 2] = -1.0f;
		glm::vec3 tangent = glm::normalize(glm::vec3(radius * nx, radius * ny, nz - height / 2.0f)
			- glm::vec3(0.0f, 0.0f, nz - height / 2.0f));
		glm::vec3 bitangent = glm::cross(tangent, glm::vec3(0.0f, 0.0f, -1.0f));
		tg[idx + 0] = tangent.x;
		tg[idx + 1] = tangent.y;
		tg[idx + 2] = tangent.z;
		bt[idx + 0] = bitangent.x;
		bt[idx + 1] = bitangent.y;
		bt[idx + 2] = bitangent.z;
		idx += 3;
		s = (-nx + 1.0f) * 0.5f;
		t = (ny + 1.0f) * 0.5f;
		tex[tIdx + 0] = s;
		tex[tIdx + 1] = t;
		tIdx += 2;
	}

	// bottom center
	p[idx + 0] = 0;
	p[idx + 1] = 0;
	p[idx + 2] = -height / 2.0f;
	n[idx + 0] = 0;
	n[idx + 1] = 0;
	n[idx + 2] = -height / 2.0f - 1.0f;
	tg[idx + 0] = 1.0f;
	tg[idx + 1] = 0.0f;
	tg[idx + 2] = -height / 2.0f;
	bt[idx + 0] = 0.0f;
	bt[idx + 1] = 1.0f;
	bt[idx + 2] = -height / 2.0f;
	idx += 3;
	tex[tIdx + 0] = 0.5;
	tex[tIdx + 1] = 0.5;
	tIdx += 2;

	// top face
	for (unsigned int i = 0; i <= nSlices; i++)
	{
		theta = i * thetaFac;		
		nx = cosf(theta);
		ny = sinf(theta);
		nz = height;

		p[idx + 0] = radius * nx;
		p[idx + 1] = radius * ny;
		p[idx + 2] = nz - height / 2.0f;
		n[idx + 0] = 0;
		n[idx + 1] = 0;
		n[idx + 2] = 1.0f;
		glm::vec3 tangent = glm::normalize(glm::vec3(radius * nx, radius * ny, nz - height / 2.0f)
			- glm::vec3(0.0f, 0.0f, nz - height / 2.0f));
		glm::vec3 bitangent = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f));
		tg[idx + 0] = tangent.x;
		tg[idx + 1] = tangent.y;
		tg[idx + 2] = tangent.z;
		bt[idx + 0] = bitangent.x;
		bt[idx + 1] = bitangent.y;
		bt[idx + 2] = bitangent.z;
		idx += 3;
		s = (nx + 1.0f) * 0.5f;
		t = (ny + 1.0f) * 0.5f;
		tex[tIdx + 0] = s;
		tex[tIdx + 1] = t;
		tIdx += 2;
	}

	// top center
	p[idx + 0] = 0;
	p[idx + 1] = 0;
	p[idx + 2] = height / 2;
	n[idx + 0] = 0;
	n[idx + 1] = 0;
	n[idx + 2] = 1.0f;
	tg[idx + 0] = 1.0f;
	tg[idx + 1] = 0.0f;
	tg[idx + 2] = height / 2.0f;
	bt[idx + 0] = 0.0f;
	bt[idx + 1] = 1.0f;
	bt[idx + 2] = height / 2.0f;

	tex[tIdx + 0] = 0.5;
	tex[tIdx + 1] = 0.5;

	// Generate the element list
	// Body
	idx = 0;
	for (unsigned int i = 0; i < nSlices; i++)
	{
		unsigned int stackStart = i * (nStacks + 1);
		unsigned int nextStackStart = (i + 1) * (nStacks + 1);
		for (unsigned int j = 0; j < nStacks; j++)
		{
			el[idx + 0] = nextStackStart + j + 1;
			el[idx + 1] = stackStart + j + 1;
			el[idx + 2] = stackStart + j;
			el[idx + 3] = nextStackStart + j + 1;
			el[idx + 5] = nextStackStart + j;
			el[idx + 4] = stackStart + j;
			idx += 6;
		}
	}

	// Bottom face
	unsigned int j = ((nSlices + 1) * (nStacks + 1));
	for (unsigned int i = 0; i < nSlices; i++, j++)
	{
		el[idx + 0] = j;
		el[idx + 1] = ((nSlices + 1) * (nStacks + 1)) + nSlices + 1;
		el[idx + 2] = j + 1;
		idx += 3;
	}

	// Top face
	j = ((nSlices + 1) * (nStacks + 1)) + (nSlices + 2);
	for (unsigned int i = 0; i < nSlices; i++, j++)
	{
		el[idx + 0] = j;
		el[idx + 1] = j + 1;
		el[idx + 2] = (((nSlices + 1) * (nStacks + 1)) + nSlices * 2) + 3;
		idx += 3;
	}

	initBuffers(&el, &p, &n, &tex, &tg, &bt);
	computeBounds();
}

TriangleMesh* Cylinder::clone()
{
	return new Cylinder(_prog, _radius, _height, _slices, _stacks, _sMax, _tMax);
}

void Cylinder::computeBounds()
{
	QList<float> xVals, yVals, zVals;
	for (size_t i = 0; i < _trsfpoints.size(); i += 3)
	{
		xVals.push_back(_trsfpoints.at(i));
		yVals.push_back(_trsfpoints.at(i + 1));
		zVals.push_back(_trsfpoints.at(i + 2));
	}
	std::sort(xVals.begin(), xVals.end(), std::less<float>());
	std::sort(yVals.begin(), yVals.end(), std::less<float>());
	std::sort(zVals.begin(), zVals.end(), std::less<float>());
	_boundingBox.setLimits(xVals.first(), xVals.last(),
		yVals.first(), yVals.last(),
		zVals.first(), zVals.last());
	Point cen = _boundingBox.center();

	_boundingSphere.setCenter(cen.getX(), cen.getY(), cen.getZ());
	_boundingSphere.setRadius(sqrt(_radius * _radius + _height / 2.0f * _height / 2.0f));
}
