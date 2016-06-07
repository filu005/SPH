/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubes.h
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	Marching Cubes Algorithm
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MARCHINGCUBES_H_
#define MARCHINGCUBES_H_

#include <glm/glm.hpp>
#include <memory>

//#include "constants.hpp"
#include "MCTable.h"

typedef struct {
	glm::vec3 p[3];
	glm::vec3 norm;
} TRIANGLE1;

//struct for storing triangle information - 3 vertices and 3 normal vectors for each vertex
typedef struct {
	glm::vec3 p[3];
	glm::vec3 norm[3];
} TRIANGLE;

//does Linear Interpolation between points p1 and p2 (they already contain their computed values)
glm::vec3 LinearInterp(const glm::vec4& p1, const glm::vec4& p2, float value);

////////////////////////////////////////////////////////////////////////////////////////
//POINTERS TO FUNCTIONS
//pointer to function which computes if point p is outside the surface
typedef bool (*OUTSIDE)(glm::vec3);

//pointer to function which determines the point of intersection of the edge with 
//the isosurface between points p1 and p2
//any other information is passed in the void array mcInfo
typedef glm::vec3(*INTERSECTION)(const glm::vec4&, const glm::vec4&, float);

//pointer to function which computes the value at point p
typedef float (*FORMULA)(glm::vec3);

///// the MARCHING CUBES algorithm itself /////

//	1A).
//takes number of cells (ncellsX, ncellsY, ncellsZ) to subdivide on each axis
// minValue is the third argument for INTERSECTION function
// array of length (ncellsX+1)(ncellsY+1)(ncellsZ+1) of mp4Vector points containing coordinates and values
// function of type mpVector (mp4Vector p1, mp4Vector p2) intersection, which determines the 
//  point of intersection of the surface and the edge between points p1 and p2
//returns pointer to triangle array and the number of triangles in numTriangles
//note: array of points is first taken on x axis, then y and then z. So for example, if u iterate through it in a
//       for loop, have indexes i, j, k for x, y, z respectively, then to get the point you will have to make the
//		 following index: i*(ncellsY+1)*(ncellsZ+1) + j*(ncellsZ+1) + k .
//		Also, the array starts at the minimum on all axes.
//TODO: another algorithm which takes array of JUST values. Coordinates then start at farthest, lower-right corner.
std::unique_ptr<TRIANGLE1[]> MarchingCubes(int ncellsX, int ncellsY, int ncellsZ, float minValue, std::unique_ptr<glm::vec4[]> points,
									INTERSECTION intersection, int &numTriangles);
//  1B).
//same as above only does linear interpolation so no INTERSECTION function is needed
std::unique_ptr<TRIANGLE1[]> MarchingCubesLinear(int ncellsX, int ncellsY, int ncellsZ, float minValue,
	std::unique_ptr<glm::vec4[]> points, int &numTriangles);

// 'STRAIGHT' Marching Cubes Algorithm //////////////////////////////////////////////////
//takes number of cells (ncellsX, ncellsY, ncellsZ) to subdivide on each axis
// minValue used to pass into LinearInterp
// gradFactor for each axis (multiplies each component of gradient vector by 1/(2*gradFactor) ).
//		Should be set to the length of a side (or close to it)
// array of length (ncellsX+1)(ncellsY+1)(ncellsZ+1) of mp4Vector points containing coordinates and values
//returns pointer to triangle array and the number of triangles in numTriangles
//note: array of points is first taken on z axis, then y and then x. So for example, if you iterate through it in a
//       for loop, have indexes i, j, k for x, y, z respectively, then to get the point you will have to make the
//		 following index: i*(ncellsY+1)*(ncellsZ+1) + j*(ncellsZ+1) + k .
//		Also, the array starts at the minimum on all axes.
std::unique_ptr<TRIANGLE[]> MarchingCubes(int ncellsX, int ncellsY, int ncellsZ, float gradFactorX, float gradFactorY, float gradFactorZ,
	float minValue, std::unique_ptr<glm::vec4[]> points, int &numTriangles);
/////////////////////////////////////////////////////////////////////////////////////////
#endif
