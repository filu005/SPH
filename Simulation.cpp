#include "Simulation.hpp"


Simulation::Simulation()
{

}

void Simulation::run(float dt)
{
	grid.clear_grid();
	particle_system.insert_sort_particles_by_indices();
	this->bin_particles_in_grid();

	//particle_system.move_particles_around(dt);
	//particle_system.reset_buffers();
}

void Simulation::bin_particles_in_grid()
{
	using particle_system::get_cell_index;

	auto & particles = particle_system.particles;
	auto & grid = this->grid.grid;

	for(auto & i : particles)
	{
		int c = get_cell_index(i.position);

		// temporary hack
		if(c < 0 || c >= c::C)
			c = 0;

		if(grid[c].no_particles == 0)
			grid[c].first_particle = &i;

		++grid[c].no_particles;
	}
}

void Simulation::iterate_through_all_neighbours()
{
	using particle_system::get_cell_index;

	auto & particles = particle_system.particles;
	auto & grid = this->grid.grid;

	// go through all grids
	for(auto& i : grid)
	{
		auto particle_i = i.first_particle;

		// go through all particles in grid [i]
		for(int ii = 0; ii < i.no_particles; ++ii)
		{
			int neighbours_count = 0;

			// go through neighbours of particle [ii] in grid [i]
			for(int z = -1; z <= 1; ++z)
			{
				for(int y = -1; y <= 1; ++y)
				{
					for(int x = -1; x <= 1; ++x)
					{
						int neighbour_grid_idx = get_cell_index(particle_i->position + glm::vec3(x*c::dx, y*c::dy, z*c::dz));
						if(neighbour_grid_idx < 0 || neighbour_grid_idx >= c::C)
						{
							// printf("%d ,, ", neighbour_grid_idx);
							continue;
						}

						auto particle_j = grid[neighbour_grid_idx].first_particle;

						for(int j = 0; j < grid[neighbour_grid_idx].no_particles; ++j)
						{
							neighbours_count++;

							++particle_j;
						}
					}
				}
			}
			// printf(":%d\n", neighbours_count);
			++particle_i;
		}
	}
}

float Simulation::W_poly6(float r_sq, float h_sq, float h)
{
	static float coefficient = 315.0f / (64.0f*c::PIf*pow(h, 9));

	return coefficient * pow(h_sq - r_sq, 3);
}

glm::vec3 Simulation::GradW_poly6(float r, float h)
{
	static float coefficient = -945.0f / (32.0f*c::PIf*pow(h, 9));

	return glm::vec3(coefficient * pow(pow(h, 2) - pow(r, 2), 2));// * r
}

// do surface tension color field
float Simulation::LapW_poly6(float r, float h)
{
	static float coefficient = -945.0f / (32.0f*c::PIf*pow(h, 9));

	return coefficient * (pow(h, 2) - pow(r, 2)) * (3.0f*pow(h, 2) - 7.0f*pow(r, 2));
}

// float Simulation::W_spiky(float r, float h)
// {
// 	if(r <= 0 || r >= h)
// 		return 0;

// 	return 15.0/(M_PI*pow(h, 6)) * (pow(h - r, 3));
// }

glm::vec3 Simulation::GradW_spiky(float r, float h)
{
	static float coefficient = -45.0f / (c::PIf*pow(h, 6));

	return glm::vec3(coefficient * pow(h - r, 2) / r);
}

// float Simulation::W_viscosity(float r, float h)
// {
// 	if(r <= 0 || r >= h)
// 		return 0;

// 	return 15.0/(2.0*M_PI*pow(h, 3)) * (-pow(r, 3)/(2.0*pow(h, 3)) + pow(r, 2)/pow(h, 2) + h/(2.0*r) - 1);
// }

float Simulation::LapW_viscosity(float r, float h)
{
	static float coefficient = 45.0f / (c::PIf*pow(h, 6));

	return coefficient * (h - r);
}

