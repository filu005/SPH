#pragma once
#include <array>

#include "Particle.hpp"

class Painter;

class Indexer
{
public:
	Indexer();

	//void paint(Painter& p) const;
	//void setup_indexer();
	void reset_buffers();
	//void move_particles_around(double dt);

	//GLuint getVAO() const { return VAO; }

	//void clear_grid();
	//void insert_sort_by_indices();
	//void bin_particles_in_grid();

	//const GLsizei bin_count;
	//const GLsizei particles_count;
	glm::vec2 translations[c::N];
	GLfloat bin_idx[c::N];

private:
	//int get_cell_index(const glm::vec3& v);
	//void print_by_position_in_particles_array(std::array<Particle, c::N>& particles);
	//void print_by_position_in_grid(std::array<GridCell, c::C>& grid);

	//std::array<GridCell, c::C> grid;// przechowuje wskaznik do pierwszej Particle oraz ich calkowita liczbe w danej komorce.
	//std::array<Particle, c::N> particles;// wszystkie posortowane (wzgledem indeksu w tablicy grid) Particle

	static const GLfloat vertex[2];
	//GLuint VAO, VBO;
	GLuint instance_VBO;
	GLuint bin_idx_VBO;
};

