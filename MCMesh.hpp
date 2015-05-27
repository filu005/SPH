#pragma once
#include <memory>
#include <array>

#include "Paintable.hpp"

struct Vertex
{
	glm::vec3 v;
	glm::vec3 n;
};

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
	void generate_mesh(std::unique_ptr<glm::vec4[]> points);

	GLsizei no_vertices;

private:
	void update_buffers();

	GLuint const vertices_buffer_size = c::C;
	std::array<Vertex, c::C> vertices_buffer;
};
