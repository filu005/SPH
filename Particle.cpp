#include "Particle.hpp"

int Particle::no_particles;

Particle::Particle()
{
	using namespace c;

	position.x = RANDOM(xmin, xmax);
	position.y = RANDOM(ymin, ymax);
	position.z = RANDOM(zmin, zmax);
	id = no_particles;
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo)
{
	position = pos;
	velocity = velo;
	density = 998.29f;
	id = no_particles;
	++no_particles;
}