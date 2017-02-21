#include "Camera.hpp"
#include "Box.hpp"
#include "Painter.hpp"
#include "BoxEditor.hpp"

#include "glm/gtx/string_cast.hpp"
#include <iostream>

BoxEditor::BoxEditor() : camera_ref(nullptr), intersection_point(0.3f, 0.1f, 0.0f)
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
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	auto ray_origin = glm::vec3(camera.Position);
	auto ray_direction = camera.create_ray(xpos, ypos);
	auto intersection = check_intersections_with_bounding_box(ray_origin, ray_direction);

	std::cout << "intersection: " << glm::to_string(intersection) << "\n";
	//auto ray_end = ray_origin + ray_direcion * intersection;

	intersection_point = intersection;

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->intersection_point), &this->intersection_point[0]);// replace data in VBO with new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec3 BoxEditor::check_intersections_with_bounding_box(glm::vec3 ray_origin, glm::vec3 ray_direction)
{
	auto const bbox = *bounding_box_ref;
	auto intersection = glm::vec3(0.0f);

	for(int i = 0; i < bbox.no_surfaces; ++i)
	{
		auto bbox_normal = bbox.surface_normals[i];
		auto dot_prod = glm::dot(ray_direction, bbox_normal);

		std::cout << "dot_prod: " << dot_prod << "\n";

		// select only walls facing towards camera && if 0 then perpendicular
		if(dot_prod <= 0.0f)
			continue;

		auto d = bbox.surface_positions[i].x + bbox.surface_positions[i].y + bbox.surface_positions[i].z;

		auto t = -(glm::dot(ray_origin, bbox_normal) + d) / dot_prod;

		if(t < 0.0f) // miss
			continue;

		auto temp_intersection = ray_origin + ray_direction*t;
		
		//if(point_in_aabb(bbox.bottom_left_back_corner, bbox.top_right_front_corner, intersection))
		if(point_in_aabb(glm::vec3(c::xmax, c::ymax, c::zmax), glm::vec3(c::xmin, c::ymin, c::zmin), temp_intersection))
			intersection = temp_intersection;
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