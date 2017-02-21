#pragma once
#include <array>

#include "constants.hpp"
#include "Paintable.hpp"


class Box : public Paintable
{
public:
	// http://www.ntu.edu.sg/home/ehchua/programming/opengl/images/Graphics3D_RHS.png
	Box() : position(0.0f)
	{
		top_right_front_corner = glm::vec3(c::xmin + 0.015f, c::ymin + 0.015f, c::zmin + 0.015f);// min
		bottom_left_back_corner = glm::vec3(c::xmax - 0.015f, c::zmax - 0.015f, c::zmax - 0.015f);// max
		surface_positions[0] = glm::vec3(bottom_left_back_corner.x, 0, 0); surface_normals[0] = glm::vec3(c::xmin, 0, 0);// left
		surface_positions[1] = glm::vec3(top_right_front_corner.x, 0, 0); surface_normals[1] = glm::vec3(c::xmax, 0, 0);// right
		surface_positions[2] = glm::vec3(0, top_right_front_corner.y, 0); surface_normals[2] = glm::vec3(0, c::ymax, 0);// top
		surface_positions[3] = glm::vec3(0, bottom_left_back_corner.y, 0); surface_normals[3] = glm::vec3(0, c::ymin, 0);// bottom
		surface_positions[4] = glm::vec3(0, 0, top_right_front_corner.z); surface_normals[4] = glm::vec3(0, 0, c::zmax);// front
		surface_positions[5] = glm::vec3(0, 0, bottom_left_back_corner.z); surface_normals[5] = glm::vec3(0, 0, c::zmin);// back

		for(auto & normal : surface_normals)
			normal = glm::normalize(normal);

		setup_buffers();
	}

	void paint(Painter& p) const override final;
	void setup_buffers() override final;


	int const static no_surfaces = 6;
	std::array<glm::vec3, no_surfaces> surface_positions;
	std::array<glm::vec3, no_surfaces> surface_normals;
	glm::vec3 top_right_front_corner;
	glm::vec3 bottom_left_back_corner;
	glm::vec3 const position;

private:
	// Geometry, instance offset array
	GLfloat const static cube_vertices[8 * 3];
	GLuint const static cube_indices[16];

	// OpenGL
	GLuint EBO;
};

// ----------------------------------------------------------------------------
