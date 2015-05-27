#include <array>

#include "Painter.hpp"
#include "ParticleSystem.hpp"
#include "MarchingCubes.h"
#include "constants.hpp"
#include "MCMesh.hpp"

MCMesh::MCMesh()
{
	
}

MCMesh::~MCMesh()
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void MCMesh::paint(Painter& p) const
{
	p.paint(*this);
}

void MCMesh::setup_buffers(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);

	glBindVertexArray(this->VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	// tworze bufor o z gory ustalonym (maksymalnym) rozmiarze
	glBufferData(GL_ARRAY_BUFFER, vertices_buffer_size * sizeof(Vertex), &(this->vertices_buffer[0]), GL_DYNAMIC_DRAW);

	// Set the vertex attribute pointers
	// glVertexAttribPointer(location = 0, vec3 = 3, dataType = GL_FLOAT, normalise = GL_FALSE, strideSize = sizeof(struct), dataBegin = 0)
	// Vertex Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 0);
	glEnableVertexAttribArray(0);
	// Normal vector position
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, n));

	glBindVertexArray(0);
}

void MCMesh::generate_mesh(std::unique_ptr<glm::vec4[]> points)
{
	using particle_system::get_cell_index;
	//initialize data to be passed to Marching Cubes
	
	//auto mc_points = make_unique<glm::vec4[]>((c::K + 1)*(c::L + 1)*(c::M + 1));
	//glm::vec3 stepSize(c::dx, c::dy, c::dz);
	//for(int i = 0; i < c::N; ++i)
	//{
	//	glm::vec4 p = points[i];

	//	if(p.w > c::surfaceThreshold)
	//	{
	//		int c = get_cell_index(glm::vec3(p));
	//		mc_points[c] = 1;
	//	}
	//	else
	//		mc_points[c] = 0;
	//}

	//for(int i = 0; i < c::K + 1; i++)
	//	for(int j = 0; j < c::L + 1; j++)
	//		for(int k = 0; k < c::M + 1; k++)
	//		{
	//			//glm::vec4 vert(MINX + i*stepSize.x, MINY + j*stepSize.y, MINZ + k*stepSize.z, 0);
	//			glm::vec4 vert(MINX + i*stepSize.x, MINY + j*stepSize.y, MINZ + k*stepSize.z, 0);
	//			vert.w = Potential2((glm::vec3) vert);

	//			mcPoints[i*(nY + 1)*(nZ + 1) + j*(nZ + 1) + k] = vert;
	//		}

	int no_triangles = 0;
	float minValue = 0.0f;

	auto triangles = MarchingCubes(c::K, c::L, c::M, c::dx, c::dy, c::dz, minValue, std::move(points), no_triangles);

	this->no_vertices = no_triangles * 3;

	for(int i = 0; i < no_triangles; ++i)
		for(int j = 0; j < 3; ++j)
		{
			vertices_buffer[i * 3 + j].v = triangles[i].p[j];
			vertices_buffer[i * 3 + j].n = triangles[i].norm[j];
		}

	update_buffers();
}

void MCMesh::update_buffers()
{
	// http://stackoverflow.com/questions/15821969/what-is-the-proper-way-to-modify-opengl-vertex-buffer
	// alternatywa: http://stackoverflow.com/questions/7256479/is-it-possible-to-in-place-resize-vbos
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * no_vertices, &this->vertices_buffer[0]);      //replace data in VBO with new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
