#include "Painter.hpp"

#include "Box.hpp"


GLfloat const Box::cube_vertices[8 * 3] =
{
	//back
	c::xmin, c::ymin, c::zmin,
	c::xmax, c::ymin, c::zmin,
	c::xmax, c::ymax, c::zmin,
	c::xmin, c::ymax, c::zmin,
	//front
	c::xmin, c::ymin, c::zmax,
	c::xmax, c::ymin, c::zmax,
	c::xmax, c::ymax, c::zmax,
	c::xmin, c::ymax, c::zmax
};

GLuint const Box::cube_indices[16] =
{
	// Back
	0, 1, 2, 3,
	// Front
	4, 5, 6, 7,
	// Middle lines
	0, 4, 1, 5, 2, 6, 3, 7
};


void Box::paint(Painter& p) const
{
	p.paint(*this);
}

void Box::setup_buffers(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);

	glBindVertexArray(this->VAO);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->cube_vertices), &this->cube_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->cube_indices), &this->cube_indices[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// - nie rob tego tutej!

	// Set the vertex attribute pointers
	// Vertex Positions
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


