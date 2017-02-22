#pragma once

#include "Paintable.hpp"
#include "SphereModel.hpp"

class ParticleSystem;
class Camera;
class Box;

class BoxEditor : public Paintable
{
public:
	BoxEditor();

	enum State
	{
		FREE_MODE,
		PLACING
	};

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	void set_camera(Camera const & camera) { camera_ref = &camera; }
	void set_bounding_box(Box const & box) { bounding_box_ref = &box; };
	void set_particle_system(ParticleSystem & ps) { particle_system_ref = &ps; };
	
	/**
	 * Currently used for placing a particle.
	 */
	void process_mouse_click(float xpos, float ypos);
	void process_mouse_movement(float xpos, float ypos);
	void process_extrusion(float extrusion);

	void switch_VBO(GLuint vbo, int no_vertices, GLuint draw_mode);
	void switch_editor_state(State s)
	{
		editor_state = s;
	}

	GLuint draw_mode;
	int no_vertices_to_draw;

	glm::vec3 intersection_point;

private:
	// non-owning pointers to other 'modules'
	Camera const * camera_ref;
	Box const * bounding_box_ref;
	ParticleSystem * particle_system_ref;
	State editor_state;

	SphereModel const sphere_model;
	GLuint sphere_VBO;

	float _extrusion;

	glm::vec3 check_intersections_with_bounding_box(glm::vec3 const ray_origin, glm::vec3 const ray_direction, glm::vec3 const camera_front);
	bool point_in_aabb(const glm::vec3 vec_max, const glm::vec3 vec_min, const glm::vec3& point);
};