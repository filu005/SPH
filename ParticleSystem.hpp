#pragma once
// Gamasutra: Building an Advanced Particle System
// http://www.gamasutra.com/view/feature/131565/building_an_advanced_particle_.php

#include <array>

#include "Particle.hpp"
#include "Paintable.hpp"


namespace particle_system
{
	int get_cell_index(const glm::vec3 v);
	bool out_of_grid_scope(const glm::vec3 v);
}

class ParticleSystem : public Paintable
{
public:
	// http://stackoverflow.com/questions/20091046/what-should-a-c-getter-return
	friend class Simulation;// jedynie do macania 'std::array<> particles'

	ParticleSystem();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void reset_buffers();
	void move_particles_around(float dt);
	void update_buffers();

	void insert_sort_particles_by_indices();

	GLsizei const bin_count = c::C;// == c::C
	GLsizei const particle_count = c::N;// == c::N

private:
	std::array<Particle, c::N> particles;// wszystkie posortowane (wzgledem indeksu w tablicy grid) Particle

	// Geometry, instance offset array
	GLfloat const static point_vertices[3];
	GLfloat const static sphere_vertices[720];//240
	GLuint const static sphere_indices[576];
	std::array<glm::mat4, c::N> model_matrices;
	GLfloat bin_idx[c::N];

	// OpenGL
	GLuint EBO;
	GLuint model_mat_VBO;
	GLuint bin_idx_VBO;
};
