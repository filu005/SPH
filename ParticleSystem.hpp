#pragma once
// Gamasutra: Building an Advanced Particle System
// http://www.gamasutra.com/view/feature/131565/building_an_advanced_particle_.php

#include <array>

#include "Particle.hpp"
#include "Paintable.hpp"


namespace particle_system
{
	int get_cell_index(const glm::vec3 v);
}

class ParticleSystem : public Paintable
{
public:
	friend class Simulation;

	ParticleSystem();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void reset_buffers();
	void move_particles_around(float dt);

	void insert_sort_particles_by_indices();

	GLsizei const bin_count = c::C;// == c::C
	GLsizei const particle_count = c::N;// == c::N

private:
	std::array<Particle, c::N> particles;// wszystkie posortowane (wzgledem indeksu w tablicy grid) Particle

	// Geometry, instance offset array
	GLfloat const static sphere_vertices[720];//294
	GLuint const static sphere_indices[576];
	glm::vec3 translations[c::N];
	GLfloat bin_idx[c::N];

	// OpenGL
	GLuint EBO;
	GLuint instance_VBO;
	GLuint bin_idx_VBO;
};
