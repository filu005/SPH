#include "Camera.hpp"
#include "Box.hpp"
#include "Painter.hpp"
#include "Emitters.hpp"
#include "ParticleSystem.hpp"
#include "BoxEditor.hpp"

#include "glm/gtx/string_cast.hpp"
#include <iostream>

BoxEditor::BoxEditor() : draw_mode(GL_POINTS), no_vertices_to_draw(1), camera_ref(nullptr), bounding_box_ref(nullptr), particle_system_ref(nullptr), intersection_point(c::xmax, c::ymax, c::zmax), sphere_model(), _extrusion(0.02f), bbox_active_normal{0.0f}
{
	setup_buffers();
}

void BoxEditor::paint(Painter& p) const
{
	p.paint(*this);
}

void BoxEditor::setup_buffers(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);

	glBindVertexArray(this->VAO);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->intersection_point), &this->intersection_point[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->sphere_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->sphere_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->sphere_model.vertices), &this->sphere_model.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Set the vertex attribute pointers
	// Vertex Positions
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void BoxEditor::process_mouse_click(float xpos, float ypos)
{
	if(editor_state == State::FREE_MODE)
	{
		switch_editor_state(State::PLACING);
		switch_VBO(sphere_VBO, sphere_model.no_vertices, GL_TRIANGLES);
		new_emitter_velocity_vector = bbox_active_normal;
	}
	else if(editor_state == State::PLACING)
	{
		auto & ps = *particle_system_ref;
		//auto & emtt = *emitters_ref;
		// add emitter; place emitter in place
		Particle particle = Particle(intersection_point, new_emitter_velocity_vector, 1, RANDOM(0.1f, 0,2f), 998.29f, c::restDensity * 1.25f, c::viscosity, c::particleMass * 1.25f, 0.5f);
		ps.add_particle(particle);  //type=1 -> adding tumor
		//emtt.add_emitter(Emitter(intersection_point, new_emitter_velocity_vector, 0, RANDOM(0.8f, 1.0f), 998.29f, c::restDensity * 1.25f, c::viscosity, c::particleMass * 1.25f, 0.5f));
		// reset extrusion
		_extrusion = 0.02f;
		// switch editor to free mode
		switch_editor_state(State::FREE_MODE);
		switch_VBO(VBO, 1, GL_POINTS);
	}
}

void BoxEditor::process_mouse_movement(float xpos, float ypos, float xoffset, float yoffset)
{
	assert(camera_ref != nullptr);
	auto const & camera = *camera_ref;

	if (editor_state == State::FREE_MODE)
	{
		auto ray_origin = glm::vec3(camera.Position);
		auto ray_direction = camera.create_ray(xpos, ypos);

		intersection_point = check_intersections_with_bounding_box(ray_origin, ray_direction, camera.Front);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->intersection_point), &this->intersection_point[0]);// replace data in VBO with new data
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else if (editor_state == State::PLACING)
	{
		this->Yaw += xoffset;
		this->Pitch += yoffset;

		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		//new_emitter_velocity_vector += glm::normalize(front);
	}
}

void BoxEditor::process_extrusion(float extrusion)
{
	if(editor_state == State::PLACING)
		_extrusion += extrusion;
}

void BoxEditor::switch_VBO(GLuint vbo, int no_vertices, GLuint draw_mode)
{
	glBindVertexArray(this->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glBindVertexArray(0);

	this->draw_mode = draw_mode;
	no_vertices_to_draw = no_vertices;
}

glm::vec3 BoxEditor::check_intersections_with_bounding_box(glm::vec3 const ray_origin, glm::vec3 const ray_direction, glm::vec3 const camera_front)
{
	auto const & bbox = *bounding_box_ref;
	auto intersection = glm::vec3(0.0f);
	bbox_active_normal = glm::vec3(0.0f);

	for(int i = 0; i < bbox.no_surfaces; ++i)
	{
		auto bbox_normal = bbox.surface_normals[i];
		auto backfacing = glm::dot(camera_front, bbox_normal);

		// select only walls facing towards camera
		if(backfacing > 0.0f)
			continue;

		auto d = std::abs(bbox.surface_positions[i].x + bbox.surface_positions[i].y + bbox.surface_positions[i].z);

		// if glm::dot(ray_direction, bbox_normal) == 0 then perpendicular
		auto t = -(glm::dot(ray_origin, bbox_normal) + d) / glm::dot(ray_direction, bbox_normal);

		if(t < 0.0f) // miss
			continue;

		auto temp_intersection = ray_origin + ray_direction*t + std::abs(_extrusion)*bbox_normal;
		
		//if(point_in_aabb(bbox.bottom_left_back_corner, bbox.top_right_front_corner, intersection))
		if (point_in_aabb(glm::vec3(c::xmax, c::ymax, c::zmax), glm::vec3(c::xmin, c::ymin, c::zmin), temp_intersection))
		{
			intersection = temp_intersection;
			bbox_active_normal = bbox_normal;
		}
	}

	return intersection;
}

bool BoxEditor::point_in_aabb(const glm::vec3 vec_max, const glm::vec3 vec_min, const glm::vec3& point)
{

	//Check if the point is less than max and greater than min
	if(point.x >= vec_min.x  && point.x <= vec_max.x &&
		point.y >= vec_min.y && point.y <= vec_max.y &&
		point.z >= vec_min.z && point.z <= vec_max.z)
	{
		return true;
	}

	//If not, then return false
	return false;

}