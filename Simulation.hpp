#pragma once
#include "ParticleSystem.hpp"
#include "Grid.hpp"
#include "Box.hpp"


class Simulation
{
public:
	Simulation();
	
	void run(float dt);

	// main components and also Paintables
	ParticleSystem particle_system;
	Box bounding_box;
	Grid grid;

private:
	void bin_particles_in_grid();
	void iterate_through_all_neighbours();

	void emit_particles();
	void compute_density();
	void compute_forces();
	void advance();
	void resolve_collisions();

	float W_poly6(float r_sq, float h_sq, float h);
	glm::vec3 GradW_poly6(float r, float h);
	float LapW_poly6(float r, float h);
	//float W_spiky(float r, float h);
	glm::vec3 GradW_spiky(float r, float h);
	//float W_viscosity(float r, float h);
	float LapW_viscosity(float r, float h);

	int particle_count;
	float mechanical_energy;
};
