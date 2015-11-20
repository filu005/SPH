#pragma once
#include <memory>
#include <array>
#include <vector>
#include <utility>

#include "perlin.hpp"

#include "Particle.hpp"
#include "Paintable.hpp"

class DistanceField : public Paintable
{
public:
	DistanceField();
	~DistanceField();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void generate_field_from_surface_particles(std::vector<Particle> const & surface_particles);

	GLuint const get_density_texture() const { return volume_texture; }
	std::pair<GLuint, GLuint> const get_front_back_color_cube_textures() const
	{
		return std::make_pair(front_volume_texture, back_volume_texture);
	}
	GLuint const get_front_back_volume_FBO() const { return front_back_volume_FBO; }

	GLsizei const voxel_field_size;// to samo co c::voxelGridDimension

private:
	GLuint generate_field();
	GLuint generate_voxel_texture3d();
	std::pair<GLuint, GLuint> const generate_front_back_color_cube_textures();
	std::array<glm::vec3, c::voxelGrid3dSize> generate_normals_for_voxel_field(std::array<GLfloat, c::voxelGrid3dSize> const & voxel_grid);
	std::array<glm::vec3, c::voxelGrid3dSize> filter_voxel_field_normals(std::array<glm::vec3, c::voxelGrid3dSize> & voxel_grid_normals);

	int get_voxel_index(glm::vec3 const v) const;
	bool out_of_voxel_grid_scope(glm::vec3 const v) const;

	std::unique_ptr<perlin::Noise3D> perlin_noise_gen;
	perlin::NoiseFactory noise_factory;

	// opengl
	GLuint volume_texture;
	GLuint front_back_volume_FBO;
	GLuint front_volume_texture, back_volume_texture;
	GLuint depth_volume_RBO;

	// data
	GLfloat const static box_data[216];
};
