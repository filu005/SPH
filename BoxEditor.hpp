#pragma once

#include "Paintable.hpp"

class Camera;
class Box;

class BoxEditor : public Paintable
{
public:
	BoxEditor();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void set_camera(Camera const & camera) { camera_ref = &camera; }
	void set_bounding_box(Box const & box) { bounding_box_ref = &box; };

	void process_mouse_click(float xpos, float ypos);

	glm::vec3 intersection_point;

private:
	Camera const * camera_ref;
	Box const * bounding_box_ref;

	glm::vec3 check_intersections_with_bounding_box(glm::vec3 ray_origin, glm::vec3 ray_direction);
	bool point_in_aabb(const glm::vec3 vec_max, const glm::vec3 vec_min, const glm::vec3& point);
};