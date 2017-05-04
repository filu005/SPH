#include "Particle.hpp"

int Particle::no_particles;

Particle::Particle() : Particle(glm::vec3(RANDOM(c::xmin, c::xmax), RANDOM(c::ymin, c::ymax), RANDOM(c::zmin, c::zmax)), glm::vec3(0.0f), c::viscosity, c::particleMass)
{

}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo) : position(pos), velocity(velo), fluid_viscosity(c::viscosity), mass(c::particleMass)
{

}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_viscosity, float particle_mass) :
	Particle(pos, velo, c::restDensity, c::viscosity, c::particleMass)
{
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_rest_density, float fluid_viscosity, float particle_mass) :
	Particle(pos, velo, fluid_rest_density, fluid_viscosity, particle_mass, -0.5f)
{
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_rest_density, float fluid_viscosity, float particle_mass, float color_value) :
	position(pos), velocity(velo), nutrient(1.0f), density(998.29f), color_value(color_value), fluid_rest_density(fluid_rest_density), fluid_viscosity(fluid_viscosity), mass(particle_mass), smoothed_color_value(0.0f), color_field_gradient_magnitude(0.0f), id(no_particles)
{
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, int type, float fluid_rest_density, float fluid_viscosity, float particle_mass) :
	position(pos), velocity(velo), type(type), nutrient(RANDOM(0.8f, 1.0f)), density(998.29f), fluid_rest_density(fluid_rest_density), fluid_viscosity(fluid_viscosity), mass(particle_mass)
{
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, int type, float nutrient, float fluid_rest_density, float fluid_viscosity, float particle_mass) :
	position(pos), velocity(velo), type(type), nutrient(nutrient), density(998.29f), fluid_rest_density(fluid_rest_density), fluid_viscosity(fluid_viscosity), mass(particle_mass)
{
	++no_particles;
}
