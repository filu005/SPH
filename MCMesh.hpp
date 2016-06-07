#pragma once
#include <memory>
#include <array>
#include <vector>

#include "Paintable.hpp"

struct Vertex
{
	glm::vec3 v;
	glm::vec3 n;
};

struct GridCell;

class MCMesh : public Paintable
{
public:
	MCMesh();
	~MCMesh();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	GLuint getNormalmap() const { return _normalmap; }
	GLuint getTexture() const { return _texture; }
	void loadTextures();

	// generuje siatke korzystajac z siatki wczystkich czasteczek
	// stworzonej w petli symulaacji.
	// siatka tworzona przy pomocy Marching Cubes.
	// po stworzeniu siatki aktualizowany jest bufor VBO na GPU
	void generate_mesh(std::array<GridCell, c::C> const & grid);

	GLsizei no_vertices;

private:
	void update_buffers();
	bool exportOBJ(const std::string& filename) const;

	GLuint const vertices_buffer_size = c::voxelGrid3dSize;
	std::array<Vertex, c::voxelGrid3dSize> vertices_buffer;
	GLuint _normalmap, _texture;
};
