#pragma once
#include <glm/glm.hpp>
#include "constants.hpp"

#define RANDOM(MIN, MAX) (MIN) + static_cast<float>(rand()) /(static_cast<float>(RAND_MAX/((MAX)-(MIN))))

struct Particle
{
	Particle();
	Particle(const glm::vec3 pos, const glm::vec3 velo);

	glm::vec3 position;
	glm::vec3 previous_position;
	glm::vec3 velocity;
	glm::vec3 acc;
	glm::vec3 force;
	float density;
	float pressure;
	float color_field_gradient_magnitude;

	int id;
	static int no_particles;
};
