#pragma once
#include <memory>
#include <array>

#include "Paintable.hpp"

struct Vertex
{
	glm::vec3 v;
	glm::vec3 n;
};

class GridCell;

class MCMesh : public Paintable
{
public:
	MCMesh();
	~MCMesh();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	// generuje siatke korzystajac z danych podanych w pierwszym argumencie
	// przy pomocy algorytmu Marching Cubes.
	// nastepnie aktualizuje bufor VBO na GPU
	void generate_mesh(std::array<GridCell, c::C> const & grid);

	GLsizei no_vertices;

private:
	void update_buffers();
	float get_isosurface_potential(const glm::vec3 p, std::array<GridCell, c::C> const & grid);

	GLuint const vertices_buffer_size = c::C;
	std::array<Vertex, c::C> vertices_buffer;
};
