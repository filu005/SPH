#include "Particle.hpp"

int Particle::no_particles;

Particle::Particle() : Particle(glm::vec3(RANDOM(c::xmin, c::xmax), RANDOM(c::ymin, c::ymax), RANDOM(c::zmin, c::zmax)), glm::vec3(0.0f), c::viscosity, c::particleMass)
{

}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo) : Particle(pos, velo, c::viscosity, c::particleMass)
{

}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_viscosity, float particle_mass) :
	position(pos), velocity(velo), nutrient(0.0f), density(998.29f), fluid_viscosity(fluid_viscosity), mass(particle_mass), color_field_gradient_magnitude(0.0f), id(no_particles)
{
	++no_particles;
}