#include "Particle.hpp"

int Particle::no_particles;

Particle::Particle() : Particle(glm::vec3(RANDOM(c::xmin, c::xmax), RANDOM(c::ymin, c::ymax), RANDOM(c::zmin, c::zmax)), glm::vec3(0.0f), 0, proliferative, 0, RANDOM(0.5*c::time_to_live, 1.5*c::time_to_live), RANDOM(0.6f,1.0f), c::restDensity * 2.0f, c::restDensity, c::viscosity, c::particleMass, 0.5f)
{
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo) : position(pos), velocity(velo), fluid_viscosity(c::viscosity), mass(c::particleMass)
{
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_viscosity, float particle_mass) :
	Particle(pos, velo, c::restDensity, c::viscosity, c::particleMass)
{
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, float fluid_rest_density, float fluid_viscosity, float particle_mass) :
	Particle(pos, velo, 0, proliferative, 0, 200, fluid_rest_density, fluid_viscosity, particle_mass)
{
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, int type, state_t state, int time_born, int time_to_live, float density, float nutrient, float fluid_rest_density, float fluid_viscosity, float particle_mass, float color_value) :
	position(pos), velocity(velo), acc(0.0f), force(0.0f), type(type), state(state), time_born(time_born), time_to_live(time_to_live), nutrient(nutrient), density(density), pressure(0.0f), color_value(color_value), fluid_rest_density(fluid_rest_density), fluid_viscosity(fluid_viscosity), mass(particle_mass), smoothed_color_value(0.0f), color_field_gradient_magnitude(0.0f), id(no_particles)
{
	if (type == 1 && state == proliferative) {  // init tumor always in proliferative state -> initial state of consumption rate 
		this->consumption_rate = c::nutrient_consumption_rate_tumor;
	}
	if (type == 0 || type == 2) {   // healty and blood vessels always have 0.0f consumption in this model
		this->consumption_rate = c::nutrient_consumption_rate_healthy;
	}
	++no_particles;
}

Particle::Particle(const glm::vec3 pos, const glm::vec3 velo, int type, state_t state, int time_born, int time_to_live, float fluid_rest_density, float fluid_viscosity, float particle_mass) :
	position(pos), velocity(velo), type(type), state(state), time_born(time_born), time_to_live(time_to_live), nutrient(RANDOM(0.8f, 1.0f)), density(c::restDensity), fluid_rest_density(fluid_rest_density), fluid_viscosity(fluid_viscosity), mass(particle_mass)
{
	if (type == 1 && state == proliferative) {   // init tumor always in proliferative state -> initial state of consumption rate 
		this->consumption_rate = c::nutrient_consumption_rate_tumor;
	}
	if (type == 0 || type == 2) {  // healty and blood vessels always have 0.0f consumption in this model
		this->consumption_rate = c::nutrient_consumption_rate_healthy;
	}
	++no_particles;
}
