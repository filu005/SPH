#include "Painter.hpp"
#include "Grid.hpp"


GLfloat const Grid::quad_vertices[8] =
{
	// Positions
	-1.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
	-1.0f, -1.0f

	//-0.1f, 0.1f,
	//0.1f, 0.1f,
	//0.1f, -0.1f,
	//-0.1f, -0.1f
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

	for(float y = c::ymin; y < c::ymax; y += c::dy)
		for(float x = c::xmin; x < c::xmax; x += c::dx)
			translations[index++] = glm::vec2(x, y);// inkrementacja nastepuje po odczytaniu wartosci indeksu

	// Store instance data in an array buffer
	glGenBuffers(1, &this->instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * c::C, &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);

	glBindVertexArray(this->VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->quad_vertices), &this->quad_vertices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*) 0);

	// Also set instance data
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(1, 1); // Tell OpenGL this is an instanced vertex attribute.
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