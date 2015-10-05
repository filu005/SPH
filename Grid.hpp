#pragma once
#include <array>

#include "constants.hpp"
#include "Paintable.hpp"


struct Particle;

struct GridCell
{
	// reference to the first Particle in a sorted list
	Particle* first_particle;
	// Particles count in the list
	int no_particles;
};

/**
 * Grid is an optimisation structure.
 * It works as a uniform grid for fast neighbour search.
 * This grid is used for implementation of (z-)index sort optimisation method,
 * which i use in my SPH fluid simulator.
 * detailed description: http://www.escience.ku.dk/research_school/phd_courses/archive/non-rigid-modeling-and-simulation-2010/slides/copenhagen_sphImplementation.pdf
 */
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
	std::array<GridCell, c::C> grid;// grid of all cells (containing all Particles)

	// Geometry, instance offset array
	GLfloat const static cube_vertices[8*3];
	GLuint const static cube_indices[16];
	std::array<glm::vec3, c::C> translations;

	// OpenGL
	GLuint instance_VBO;
	GLuint EBO;
};
