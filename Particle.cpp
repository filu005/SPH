#include "Particle.hpp"

int Particle::no_particles;

Particle::Particle()
{
	using namespace c;

	position.x = RANDOM(xmin, xmax);
	position.y = RANDOM(ymin, ymax);
	position.z = RANDOM(zmin, zmax);
	nutrient = 0.0f;
	color_field_gradient_magnitude = 0.0f;
	id = no_particles;
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo)
{
	position = pos;
	velocity = velo;
	nutrient = 0.0f;
	density = 998.29f;
	color_field_gradient_magnitude = 0.0f;
	id = no_particles;
	++no_particles;
}