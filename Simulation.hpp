#pragma once
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

#include "ParticleSystem.hpp"
#include "DistanceField.hpp"
#include "Skybox.hpp"
#include "MCMesh.hpp"
#include "Grid.hpp"
#include "Box.hpp"


class Simulation
{
public:
	Simulation();
	~Simulation();
	
	void run(float dt);

	// main components and also Paintables
	Skybox skybox;
	ParticleSystem particle_system;
	DistanceField distance_field;
	MCMesh mesh;
	Box bounding_box;
	Grid grid;

private:
	void bin_particles_in_grid();
	void iterate_through_all_neighbours();
	std::vector<Particle> extract_surface_particles();
	std::vector<Particle> extract_surface_particles_2();

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
	glm::vec3 Grad_BicubicSpline(glm::vec3 x, float h);

	int particle_count;
	float mechanical_energy;
	std::chrono::high_resolution_clock::time_point start_time;
	std::vector<std::pair<float, float> > energy_stats;
	std::ofstream stats_file;
};

// only for stats output
namespace
{
	std::string statsToString(const std::pair<float, float>& data)
	{
		std::ostringstream str;
		str << data.first << ", " << data.second;
		return str.str();
	}
} // namespace anonymous
