#pragma once
#include <glm/glm.hpp>
#include "constants.hpp"

#define RANDOM(MIN, MAX) (MIN) + static_cast<float>(rand()) /(static_cast<float>(RAND_MAX/((MAX)-(MIN))))

struct Particle
{
	Particle();
	Particle(const glm::vec3 pos, const glm::vec3 velo);
	Particle(const glm::vec3 pos, const glm::vec3 velo, int type, float fluid_rest_density, float fluid_viscosity, float particle_mass);
	Particle(const glm::vec3 pos, const glm::vec3 velo, int type,  float density, float nutrient, float fluid_rest_density, float fluid_viscosity, float particle_mass, float color_value);

	void add_nutrient(float nut)
	{
		nutrient += nut;
	}

	glm::vec3 position;
	//glm::vec3 previous_position;
	glm::vec3 velocity;
	//glm::vec3 eval_velocity;
	glm::vec3 acc;
	glm::vec3 force;
	int type;   // 0 - healthy, 1 - tumor, 2 - blood vessel
	float nutrient;
	float new_nutrient;
	float density;
	float pressure;
	float color_value;
	float fluid_rest_density;
	float fluid_viscosity;
	float mass;
	float smoothed_color_value;
	float color_field_gradient_magnitude;
	bool at_surface;

	int id;
	static int no_particles;
};
