#pragma once
#include <memory>
#include <array>
#include <utility>

#include "perlin.hpp"

#include "Paintable.hpp"

class DistanceField : public Paintable
{
public:
	DistanceField();
	~DistanceField();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	GLuint const get_density_texture() const { return volume_texture; }
	std::pair<GLuint, GLuint> const get_front_back_color_cube_textures() const
	{
		return std::make_pair(front_volume_texture, back_volume_texture);
	}
	GLuint const get_front_back_volume_FBO() const { return front_back_volume_FBO; }

	GLsizei const voxel_field_size;

private:
	GLuint generate_field();
	std::pair<GLuint, GLuint> const generate_front_back_color_cube_textures();

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
