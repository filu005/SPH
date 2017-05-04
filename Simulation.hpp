#pragma once
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <functional>

#include "ParticleSystem.hpp"
#include "DistanceField.hpp"
#include "Skybox.hpp"
#include "MCMesh.hpp"
#include "Grid.hpp"
#include "Box.hpp"
#include "Emitters.hpp"

/**
 * Basicly main class where all computation takes place.
 * All Paintable components are kept here.
 * @param skybox	Kept here only to be consistent about Paintables - they all are here.
 	Used in painting horizont and for easy reflections/refractions (see shaders).
 * @param particle_system	Object responsible for storing and managing particles in memory.
 	Sorts particles in memory according to their grid index (or alternatively z-index).
	Also contains mesh of particles (spheres).
 * @param distance_field	Creator of 3D scalar field describing minimum distance towards (fluid) surface.
 * @param mesh	Generates a mesh by running standard Marching Cubes on previously detected surface particles.
 	Also saves mesh as OBJ.
 * @param bounding_box	Container kept here for easy access while painting and for colisions.
 * @param grid	Structure stores a 3D grid used for neighbour search optimization (see ParticleSystem).
 */
class Simulation
{
public:
	Simulation();
	~Simulation();
	
	void run(float dt);

	// main components and also Paintables
	Skybox skybox;
	ParticleSystem particle_system;
	Emitters emitters;
	DistanceField distance_field;
	MCMesh mesh;
	Box bounding_box;
	Grid grid;

private:
	/**
	* Assigns a bin index in 3D grid to every particle.
	* This method is kept here because of interdependence of grid and particle_system:
	* grid is kept in Grid structure and all particles are stored in ParticleSystem.
	*/
	void bin_particles_in_grid();

	/**
	 * extract_surface_particles(), extract_surface_particles_2():
	 * Temporary solution. Extracts surface particles. Used for ray casting.
	 */
	std::vector<Particle> extract_surface_particles();
	std::vector<Particle> extract_surface_particles_2();

	void emit_particles();
	void compute_nutrient_concentration();
	void compute_density();
	template<typename Args, typename Func0, typename Func1, typename Func2>
	void iterate_particles_traverse_neighbours(Func0 init_iparticle, Func1 computation_on_jparticles, Func2 computation_on_iparticle);
	void compute_interface_factor();
	void compute_forces_compact();
	void compute_forces();
	void advance();
	void resolve_collisions();
	void multiply_particles();

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
