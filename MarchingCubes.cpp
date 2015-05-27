/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubes.cpp
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	Marching Cubes Algorithm
/////////////////////////////////////////////////////////////////////////////////////////////

#include <glm/gtx/fast_square_root.hpp> // fastNormalize

#include "MarchingCubes.h"

glm::vec3 LinearInterp(const glm::vec4& p1, const glm::vec4& p2, float value)
{
	glm::vec3 p, pp1(p1), pp2(p2);
	//if(p1.w != p2.w)
	if(fabs(p1.w - p2.w) > 0.00001)
		p = pp1 + (pp2 - pp1)/(p2.w - p1.w)*(value - p1.w);
	else 
		p = pp1;

	return p;
}


//Macros used to compute gradient vector on each vertex of a cube
//argument should be the name of array of vertices
//can be verts or *verts if done by reference
#define CALC_GRAD_VERT_0(verts) glm::vec4(points[ind-YtimeZ].w-(verts[1]).w,points[ind-pointsZ].w-(verts[4]).w,points[ind-1].w-(verts[3]).w, (verts[0]).w);
#define CALC_GRAD_VERT_1(verts) glm::vec4((verts[0]).w-points[ind+2*YtimeZ].w,points[ind+YtimeZ-pointsZ].w-(verts[5]).w,points[ind+YtimeZ-1].w-(verts[2]).w, (verts[1]).w);
#define CALC_GRAD_VERT_2(verts) glm::vec4((verts[3]).w-points[ind+2*YtimeZ+1].w,points[ind+YtimeZ-ncellsZ].w-(verts[6]).w,(verts[1]).w-points[ind+YtimeZ+2].w, (verts[2]).w);
#define CALC_GRAD_VERT_3(verts) glm::vec4(points[ind-YtimeZ+1].w-(verts[2]).w,points[ind-ncellsZ].w-(verts[7]).w,(verts[0]).w-points[ind+2].w, (verts[3]).w);
#define CALC_GRAD_VERT_4(verts) glm::vec4(points[ind-YtimeZ+ncellsZ+1].w-(verts[5]).w,(verts[0]).w-points[ind+2*pointsZ].w,points[ind+ncellsZ].w-(verts[7]).w, (verts[4]).w);
#define CALC_GRAD_VERT_5(verts) glm::vec4((verts[4]).w-points[ind+2*YtimeZ+ncellsZ+1].w,(verts[1]).w-points[ind+YtimeZ+2*pointsZ].w,points[ind+YtimeZ+ncellsZ].w-(verts[6]).w, (verts[5]).w);
#define CALC_GRAD_VERT_6(verts) glm::vec4((verts[7]).w-points[ind+2*YtimeZ+ncellsZ+2].w,(verts[2]).w-points[ind+YtimeZ+2*ncellsZ+3].w,(verts[5]).w-points[ind+YtimeZ+ncellsZ+3].w, (verts[6]).w);
#define CALC_GRAD_VERT_7(verts) glm::vec4(points[ind-YtimeZ+ncellsZ+2].w-(verts[6]).w,(verts[3]).w-points[ind+2*ncellsZ+3].w,(verts[4]).w-points[ind+ncellsZ+3].w, (verts[7]).w);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL //
//Global variables - so they dont have to be passed into functions
int pointsZ;	//number of points on Z zxis (equal to ncellsZ+1)
int YtimeZ;		//'plane' of cubes on YZ (equal to (ncellsY+1)*pointsZ )
///////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
//	MARCHING CUBES	//

//  VERSION  1A).  //
TRIANGLE1* MarchingCubes(int ncellsX, int ncellsY, int ncellsZ, float minValue, glm::vec4 * points,
										INTERSECTION intersection, int &numTriangles)
{
	TRIANGLE1 * triangles = new TRIANGLE1[3*ncellsX*ncellsY*ncellsZ];//this should be enough space, if not change 4 to 5
	numTriangles = int(0);

	int YtimeZ = (ncellsY+1)*(ncellsZ+1);
	//go through all the points
	for(int i=0; i < ncellsX; i++)			//x axis
		for(int j=0; j < ncellsY; j++)		//y axis
			for(int k=0; k < ncellsZ; k++)	//z axis
			{
				//initialize vertices
				// przyporzadkuj wartosci kazdemu z wiercholkow budujacych szescian
				glm::vec4 verts[8];
				int ind = i*YtimeZ + j*(ncellsZ+1) + k;
   /*(step 3)*/ verts[0] = points[ind];
				verts[1] = points[ind + YtimeZ];
				verts[2] = points[ind + YtimeZ + 1];
				verts[3] = points[ind + 1];
				verts[4] = points[ind + (ncellsZ+1)];
				verts[5] = points[ind + YtimeZ + (ncellsZ+1)];
				verts[6] = points[ind + YtimeZ + (ncellsZ+1) + 1];
				verts[7] = points[ind + (ncellsZ+1) + 1];
				
				//get the index
				// sprawdz gdzie znajduja sie wierzcholki szescianu wzgledem (szukanej) powierzchni
				// dla kazdego wierzcholka, ktory znajduje sie wewnatrz powierzchni dodaj bit do 'cubeIndex'.
				int cubeIndex = int(0);
				for(int n=0; n < 8; n++)
   /*(step 4)*/		if(verts[n].w <= minValue) cubeIndex |= (1 << n);
				
				// policzony powyzej 'cubeIndex' oznacza kombinacje krawedzi, ktore przecinaja szescina (wszystkich kombinacji jest 256).

				//check if its completely inside or outside
				// jesli szescian jest w calosci 'w' lub 'poza' powierzchnia (wartosc 0 w tablicy edgeTable) 
				// to olej go i przejdz do nastepnego szascianu.
   /*(step 5)*/ if(!edgeTable[cubeIndex]) continue;
			
				// jesli szescian jest przecinany przez szukana powierzchnie to znajdujemy
				// (tutaj poslugujac sie interpolacja liniowa) punkt przeciecia na konkretnej krawedzi.
				// tablica intVerts reprezentuje kazda z krawedzi szescianu
				// (gdzie na kazdej z krawedzi moze znalezc sie punkt przeciecia).
				//get intersection vertices on edges and save into the array
   				glm::vec3 intVerts[12];
   /*(step 6)*/ if(edgeTable[cubeIndex] & 1) intVerts[0] = intersection(verts[0], verts[1], minValue);
				if(edgeTable[cubeIndex] & 2) intVerts[1] = intersection(verts[1], verts[2], minValue);
				if(edgeTable[cubeIndex] & 4) intVerts[2] = intersection(verts[2], verts[3], minValue);
				if(edgeTable[cubeIndex] & 8) intVerts[3] = intersection(verts[3], verts[0], minValue);
				if(edgeTable[cubeIndex] & 16) intVerts[4] = intersection(verts[4], verts[5], minValue);
				if(edgeTable[cubeIndex] & 32) intVerts[5] = intersection(verts[5], verts[6], minValue);
				if(edgeTable[cubeIndex] & 64) intVerts[6] = intersection(verts[6], verts[7], minValue);
				if(edgeTable[cubeIndex] & 128) intVerts[7] = intersection(verts[7], verts[4], minValue);
				if(edgeTable[cubeIndex] & 256) intVerts[8] = intersection(verts[0], verts[4], minValue);
				if(edgeTable[cubeIndex] & 512) intVerts[9] = intersection(verts[1], verts[5], minValue);
				if(edgeTable[cubeIndex] & 1024) intVerts[10] = intersection(verts[2], verts[6], minValue);
				if(edgeTable[cubeIndex] & 2048) intVerts[11] = intersection(verts[3], verts[7], minValue);

				// tablica triTable reprezentuje wszystkie mozliwe trojkaty, ktore 'buduja' powierzchnie przecinajaca szescian.
				// wartosci zwracane przez triTable to indeksy dla tablicy punktow (intVerts) z punktu powyzej.
				//now build the triangles using triTable
				//for example triTable[3][0] will be the index of the first vertex of a first triangle of the third possibility
				for (int n=0; triTable[cubeIndex][n] != -1; n+=3) {
   /*(step 7)*/ 	triangles[numTriangles].p[0] = intVerts[triTable[cubeIndex][n+2]];
					triangles[numTriangles].p[1] = intVerts[triTable[cubeIndex][n+1]];
					triangles[numTriangles].p[2] = intVerts[triTable[cubeIndex][n]];
   /*(step 8)*/ 	triangles[numTriangles].norm = glm::normalize(glm::cross(triangles[numTriangles].p[1] - triangles[numTriangles].p[0],
																			 triangles[numTriangles].p[2] - triangles[numTriangles].p[0]));
					numTriangles++;
				}
			}	//END OF FOR LOOP
		
		//free all the wasted space
		TRIANGLE1 * retTriangles = new TRIANGLE1[numTriangles];
		for(int i=0; i < numTriangles; i++)
			retTriangles[i] = triangles[i];
		delete [] triangles;
	
	return retTriangles;
}


//	VERSION  1B).  //
TRIANGLE1* MarchingCubesLinear(int ncellsX, int ncellsY, int ncellsZ, float minValue, 
									glm::vec4 * points, int &numTriangles)
{
	return MarchingCubes(ncellsX, ncellsY, ncellsZ, minValue, points, LinearInterp, numTriangles);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//	'STRAIGHT' MARCHING CUBES	ALGORITHM  ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//for gradients at the edges values of 1.0, 1.0, 1.0, 1.0  are given
std::unique_ptr<TRIANGLE[]> MarchingCubes(int ncellsX, int ncellsY, int ncellsZ,
	float gradFactorX, float gradFactorY, float gradFactorZ,
	float minValue, std::unique_ptr<glm::vec4[]> points, int &numTriangles)
{
	//this should be enough space, if not change 3 to 4
	TRIANGLE * triangles = new TRIANGLE[3 * ncellsX*ncellsY*ncellsZ];
	numTriangles = int(0);
	
	pointsZ = ncellsZ + 1;			//initialize global variable (for extra speed) 
	YtimeZ = (ncellsY + 1)*pointsZ;
	int lastX = ncellsX;			//left from older version
	int lastY = ncellsY;
	int lastZ = ncellsZ;

	glm::vec4 *verts[8];			//vertices of a cube (array of pointers for extra speed)
	glm::vec3 intVerts[12];			//linearly interpolated vertices on each edge
	int cubeIndex;					//shows which vertices are outside/inside
	int edgeIndex;					//index returned by edgeTable[cubeIndex]
	glm::vec4 gradVerts[8];			//gradients at each vertex of a cube		
	glm::vec3 grads[12];				//linearly interpolated gradients on each edge
	int indGrad;					//shows which gradients already have been computed
	int ind, ni, nj;				//ind: index of vertex 0
	//factor by which corresponding coordinates of gradient vectors are scaled
	glm::vec3 factor(1.0 / (2.0*gradFactorX), 1.0 / (2.0*gradFactorY), 1.0 / (2.0*gradFactorZ));

	//DebugWrite("1");

	//MAIN LOOP: goes through all the points
	for(int i = 0; i < lastX; i++)
	{			//x axis
		ni = i*YtimeZ;
		for(int j = 0; j < lastY; j++)
		{		//y axis
			nj = j*pointsZ;
			for(int k = 0; k < lastZ; k++, ind++)	//z axis
			{
				//initialize vertices
				ind = ni + nj + k;
				verts[0] = &points[ind];
				verts[1] = &points[ind + YtimeZ];
				verts[4] = &points[ind + pointsZ];
				verts[5] = &points[ind + YtimeZ + pointsZ];
				verts[2] = &points[ind + YtimeZ + 1];
				verts[3] = &points[ind + 1];
				verts[6] = &points[ind + YtimeZ + pointsZ + 1];
				verts[7] = &points[ind + pointsZ + 1];

				//get the index
				cubeIndex = int(0);
				for(int n = 0; n < 8; n++)
				if(verts[n]->w <= minValue) cubeIndex |= (1 << n);

				//check if its completely inside or outside
				if(!edgeTable[cubeIndex]) continue;

				indGrad = int(0);
				edgeIndex = edgeTable[cubeIndex];

				// http://www.angelfire.com/linux/myp/MCAdvanced/gformula.gif
				if(edgeIndex & 1) 
				{
					intVerts[0] = LinearInterp(*verts[0], *verts[1], minValue);
					if(i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
					else gradVerts[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					if(i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
					else gradVerts[1] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					indGrad |= 3;
					grads[0] = LinearInterp(gradVerts[0], gradVerts[1], minValue);
					grads[0].x *= factor.x; grads[0].y *= factor.y; grads[0].z *= factor.z;
				}
				if(edgeIndex & 2) 
				{
					intVerts[1] = LinearInterp(*verts[1], *verts[2], minValue);
					if(!(indGrad & 2)) 
					{
						if(i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
						else gradVerts[1] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 2;
					}
					if(i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
					else gradVerts[2] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					indGrad |= 4;
					grads[1] = LinearInterp(gradVerts[1], gradVerts[2], minValue);
					grads[1].x *= factor.x; grads[1].y *= factor.y; grads[1].z *= factor.z;
				}
				if(edgeIndex & 4) 
				{
					intVerts[2] = LinearInterp(*verts[2], *verts[3], minValue);
					if(!(indGrad & 4)) 
					{
						if(i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
						else gradVerts[2] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 4;
					}
					if(i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
					else gradVerts[3] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					indGrad |= 8;
					grads[2] = LinearInterp(gradVerts[2], gradVerts[3], minValue);
					grads[2].x *= factor.x; grads[2].y *= factor.y; grads[2].z *= factor.z;
				}
				if(edgeIndex & 8) 
				{
					intVerts[3] = LinearInterp(*verts[3], *verts[0], minValue);
					if(!(indGrad & 8))
					{
						if(i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
						else gradVerts[3] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 8;
					}
					if(!(indGrad & 1)) 
					{
						if(i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
						else gradVerts[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 1;
					}
					grads[3] = LinearInterp(gradVerts[3], gradVerts[0], minValue);
					grads[3].x *= factor.x; grads[3].y *= factor.y; grads[3].z *= factor.z;
				}
				if(edgeIndex & 16) 
				{
					intVerts[4] = LinearInterp(*verts[4], *verts[5], minValue);

					if(i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
					else gradVerts[4] = glm::vec4(1.0, 1.0, 1.0, 1.0);

					if(i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
					else gradVerts[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);

					indGrad |= 48;
					grads[4] = LinearInterp(gradVerts[4], gradVerts[5], minValue);
					grads[4].x *= factor.x; grads[4].y *= factor.y; grads[4].z *= factor.z;
				}
				if(edgeIndex & 32) 
				{
					intVerts[5] = LinearInterp(*verts[5], *verts[6], minValue);
					if(!(indGrad & 32)) {
						if(i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
						else gradVerts[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 32;
					}

					if(i != lastX - 1 && j != lastY - 1 && k != lastZ - 1)
						gradVerts[6] = CALC_GRAD_VERT_6(*verts)
					else
						gradVerts[6] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					indGrad |= 64;
					grads[5] = LinearInterp(gradVerts[5], gradVerts[6], minValue);
					grads[5].x *= factor.x; grads[5].y *= factor.y; grads[5].z *= factor.z;
				}
				if(edgeIndex & 64) 
				{
					intVerts[6] = LinearInterp(*verts[6], *verts[7], minValue);
					if(!(indGrad & 64)) {
						if(i != lastX - 1 && j != lastY - 1 && k != lastZ - 1)
							gradVerts[6] = CALC_GRAD_VERT_6(*verts)
						else
							gradVerts[6] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 64;
					}

					if(i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
					else gradVerts[7] = glm::vec4(1.0, 1.0, 1.0, 1.0);
					indGrad |= 128;
					grads[6] = LinearInterp(gradVerts[6], gradVerts[7], minValue);
					grads[6].x *= factor.x; grads[6].y *= factor.y; grads[6].z *= factor.z;
				}
				if(edgeIndex & 128) 
				{
					intVerts[7] = LinearInterp(*verts[7], *verts[4], minValue);
					if(!(indGrad & 128)) 
					{
						if(i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
						else gradVerts[7] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 128;
					}
					if(!(indGrad & 16)) 
					{
						if(i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
						else gradVerts[4] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 16;
					}
					grads[7] = LinearInterp(gradVerts[7], gradVerts[4], minValue);
					grads[7].x *= factor.x; grads[7].y *= factor.y; grads[7].z *= factor.z;
				}
				if(edgeIndex & 256) 
				{
					intVerts[8] = LinearInterp(*verts[0], *verts[4], minValue);
					if(!(indGrad & 1)) {
						if(i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
						else gradVerts[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 1;
					}
					if(!(indGrad & 16)) 
					{
						if(i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
						else gradVerts[4] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 16;
					}
					grads[8] = LinearInterp(gradVerts[0], gradVerts[4], minValue);
					grads[8].x *= factor.x; grads[8].y *= factor.y; grads[8].z *= factor.z;
				}
				if(edgeIndex & 512) 
				{
					intVerts[9] = LinearInterp(*verts[1], *verts[5], minValue);
					if(!(indGrad & 2)) {
						if(i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
						else gradVerts[1] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 2;
					}
					if(!(indGrad & 32))
					{
						if(i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
						else gradVerts[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 32;
					}
					grads[9] = LinearInterp(gradVerts[1], gradVerts[5], minValue);
					grads[9].x *= factor.x; grads[9].y *= factor.y; grads[9].z *= factor.z;
				}
				if(edgeIndex & 1024)
				{
					intVerts[10] = LinearInterp(*verts[2], *verts[6], minValue);
					if(!(indGrad & 4))
					{
						if(i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
						else gradVerts[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 4;
					}
					if(!(indGrad & 64))
					{
						if(i != lastX - 1 && j != lastY - 1 && k != lastZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
						else gradVerts[6] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 64;
					}
					grads[10] = LinearInterp(gradVerts[2], gradVerts[6], minValue);
					grads[10].x *= factor.x; grads[10].y *= factor.y; grads[10].z *= factor.z;
				}
				if(edgeIndex & 2048)
				{
					intVerts[11] = LinearInterp(*verts[3], *verts[7], minValue);
					if(!(indGrad & 8))
					{
						if(i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
						else gradVerts[3] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 8;
					}
					if(!(indGrad & 128))
					{
						if(i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
						else gradVerts[7] = glm::vec4(1.0, 1.0, 1.0, 1.0);
						indGrad |= 128;
					}
					grads[11] = LinearInterp(gradVerts[3], gradVerts[7], minValue);
					grads[11].x *= factor.x; grads[11].y *= factor.y; grads[11].z *= factor.z;
				}

				//DebugWrite("2");

				//now build the triangles using triTable
				for(int n = 0; triTable[cubeIndex][n] != -1; n += 3)
				{
					int index[3] = { triTable[cubeIndex][n + 2], triTable[cubeIndex][n + 1], triTable[cubeIndex][n] };
					for(int h = 0; h < 3; h++)
					{	//copy vertices and normals into triangles array
						triangles[numTriangles].p[h] = intVerts[index[h]];
						triangles[numTriangles].norm[h] = grads[index[h]];
					}
					numTriangles++;	//one more triangle has been added
				}
			}	//END OF FOR LOOP ON Z AXIS
		}	//END OF FOR LOOP ON Y AXIS
	}	//END OF FOR LOOP ON X AXIS

	//DebugWrite("3");

	//free all wasted space
	auto retTriangles = std::make_unique<TRIANGLE[]>(numTriangles);
	for(int i = 0; i < numTriangles; i++)
		retTriangles[i] = triangles[i];
	delete[] triangles;

	return retTriangles;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
