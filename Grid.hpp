#pragma once
#include <array>

#include "constants.hpp"
#include "Paintable.hpp"


struct Particle;

struct GridCell
{
	Particle* first_particle;
	int no_particles;
};

class Grid : public Paintable
{
public:
	friend class Simulation;

	Grid();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void clear_grid();

	GLsizei const bin_count = c::C;// == c::C

private:
	// Hot stuff
	std::array<GridCell, c::C> grid;// przechowuje wskaznik do pierwszej Particle oraz ich calkowita liczbe w danej komorce.

	// Geometry, instance offset array
	GLfloat const static cube_vertices[8*3];
	GLuint const static cube_indices[16];
	std::array<glm::vec3, c::C> translations;

	// OpenGL
	GLuint instance_VBO;
	GLuint EBO;
};
