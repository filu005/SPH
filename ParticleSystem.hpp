#pragma once
// Gamasutra: Building an Advanced Particle System
// http://www.gamasutra.com/view/feature/131565/building_an_advanced_particle_.php

#include <array>
#include <memory>

#include "Particle.hpp"
#include "Paintable.hpp"


namespace particle_system
{
	// returns an index of bin (cell) in Grid in 3D coordinates
	inline int get_cell_index(const glm::vec3 v);
	glm::ivec3 get_grid_coords(glm::vec3 const v);
	glm::vec3 get_grid_coords_in_real_system(glm::vec3 const v);
	bool out_of_grid_scope(const glm::vec3 v);
	inline uint64_t get_z_index(glm::ivec3 const v);
	inline uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z);
	inline uint64_t splitBy3(unsigned int a);
}

/**
 * Stores and sorts (according to bin [cell] index) particles.
 * Does NOT place particles into bins (see Simulation::bin_particles_in_grid()).
 */
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
	std::unique_ptr<glm::vec4[]> get_position_color_field_data();

	/**
	 * Sorts particles by a cell (bin) index. cell is an elementary part
	 * of Grid. Thanks to sorting the Grid can easily store an information about neighbours.
	 */
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
	GLuint surface_particles[c::N];

	// OpenGL
	GLuint EBO;
	GLuint model_mat_VBO;
	GLuint bin_idx_VBO;
	GLuint at_surface_VBO;
};
