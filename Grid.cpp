#include "Painter.hpp"
#include "Grid.hpp"


GLfloat const Grid::cube_vertices[8*3] =
{
	0.0f, 0.0f, 0.0f,
	c::dx, 0.0f, 0.0f,
	c::dx, c::dy, 0.0f,
	0.0f, c::dy, 0.0f,
	0.0f, 0.0f, c::dz,
	c::dx, 0.0f, c::dz,
	c::dx, c::dy, c::dz,
	0.0f, c::dy, c::dz
};

GLuint const Grid::cube_indices[16] =
{
	// Front
	0, 1, 2, 3,
	// Back
	4, 5, 6, 7,
	// Middle lines
	0, 4, 1, 5, 2, 6, 3, 7
};

Grid::Grid()
{
	setup_buffers();
}

void Grid::paint(Painter& p) const
{
	p.paint(*this);
}

void Grid::setup_buffers(void)
{
	int index = 0;

	for(float z = c::zmin; z < c::zmax; z += c::dz)
		for(float y = c::ymin; y < c::ymax; y += c::dy)
			for(float x = c::xmin; x < c::xmax; x += c::dx)
				translations[index++] = glm::vec3(x, y, z);// post-increment

	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);

	glBindVertexArray(this->VAO);

	// Store instance data in an array buffer
	glGenBuffers(1, &this->instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * c::C, &this->translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->cube_vertices), &this->cube_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->cube_indices), &this->cube_indices[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// - no no no!

	// Set the vertex attribute pointers
	// Vertex Positions
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

	// Also set instance data
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glVertexAttribDivisor(1, 1); // Tell OpenGL this is an instanced vertex attribute.

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Grid::clear_grid()
{
	std::for_each(std::begin(grid), std::end(grid), [&](GridCell& c)
	{
		c = { nullptr, 0 };
	});
	//for(auto& c : grid)
	//	c = { nullptr, 0 };
}