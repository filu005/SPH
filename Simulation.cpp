#include "Simulation.hpp"


Simulation::Simulation() : particle_count(0), mechanical_energy(0.0f), stats_file("./../plot/wydajnosc/perf(t) " + std::to_string(c::K) + ".txt")
{
	start_time = std::chrono::high_resolution_clock::now();
}

Simulation::~Simulation()
{
	// from http://stackoverflow.com/questions/634087/stdcopy-to-stdcout-for-stdpair
	//std::ostream_iterator<std::string> output_iterator(stats_file, "\n");
	//std::transform(
	//	energy_stats.begin(),
	//	energy_stats.end(),
	//	std::ostream_iterator<std::string>(stats_file, "\n"),
	//	statsToString);
}

void Simulation::run(float dt)
{
	emit_particles();

	grid.clear_grid();
	particle_system.insert_sort_particles_by_indices();
	bin_particles_in_grid();

	compute_density();
	compute_forces();
	resolve_collisions();
	advance();

	// tutaj bo Painter::paint() jest const
	// do wizualizacji:
	// za pomoca siatki generowanej przez MC
	mesh.generate_mesh(grid.grid);
	// przy pomocy ray castingu na distance field
	//distance_field.generate_field_from_surface_particles(extract_surface_particles());
	// wizualizacja poszczegolnych czasteczek
	particle_system.update_buffers();
}

void Simulation::bin_particles_in_grid()
{
	using particle_system::get_cell_index;
	using particle_system::out_of_grid_scope;
	using particle_system::get_grid_coords;
	using particle_system::get_z_index;

	auto & particles = particle_system.particles;
	auto & grid = this->grid.grid;

	for(auto & i : particles)
	{
		// rozwiazanie na kiedy uzyskam dobrze dzialajacy mechanizm kolizji (gdzie czasteczki nie beda wypadac na orbite/poza 'pudelko')
		glm::vec3 particle_position_vector = i.position;
		if(out_of_grid_scope(particle_position_vector))
			continue;
		int c = get_cell_index(particle_position_vector);//sorted_particles_cell_indexes
		//int c = get_z_index(get_grid_coords(particle_position_vector));

		//int c = get_cell_index(i.position);
		//// temporary hack
		//if(c < 0 || c >= c::C)
		//	continue;

		if(grid[c].no_particles == 0)
			grid[c].first_particle = &i;

		++grid[c].no_particles;
	}
}

std::vector<Particle> Simulation::extract_surface_particles()
{
	using particle_system::get_cell_index;
	using particle_system::out_of_grid_scope;
	using particle_system::get_grid_coords_in_real_system;
	using namespace c;

	auto & grid = this->grid.grid;

	std::vector<Particle> surface_particles;
	std::vector<float> threshold_values;
	surface_particles.reserve(static_cast<unsigned int>(c::N * 0.5f));

	// go through all grids
	for(auto & i : grid)
	{
		Particle * particle_i_ptr = i.first_particle;

		// go through all particles in grid [i]
		for(int ii = 0; ii < i.no_particles; ++ii)
		{
			Particle & particle_i = *particle_i_ptr;
			glm::vec3 center_mass_distance{ 0.f };
			auto neighbourhood_centre = get_grid_coords_in_real_system(particle_i.position) + glm::vec3(c::dx*0.5f, c::dy*0.5f, c::dz*0.5f);
			glm::vec3 mass_x_position_sum{ 0.f };
			auto mass_sum = 0.f;
			auto neighbourhood_no = 0u;

			// go through neighbours of particle [ii] in grid [i]
			for(int z = -1; z <= 1; ++z)
			{
				for(int y = -1; y <= 1; ++y)
				{
					for(int x = -1; x <= 1; ++x)
					{
						glm::vec3 neighbour_cell_vector = particle_i.position + glm::vec3(x*c::dx, y*c::dy, z*c::dz);
						//assert(!out_of_grid_scope(neighbour_cell_vector) && "jezus maria jakas czasteczka wyskoczyla!");
						if(out_of_grid_scope(neighbour_cell_vector))
							continue;

						int neighbour_grid_idx = get_cell_index(neighbour_cell_vector);

						Particle * particle_j_ptr = grid[neighbour_grid_idx].first_particle;

						for(int j = 0; j < grid[neighbour_grid_idx].no_particles; ++j)
						{
							Particle& particle_j = *particle_j_ptr;
							// wydaje mi sie ze position_j_in_neighbourhood powinno byc potraktowane glm::abs()
							// ale liczac bez wartosci bezwzglednej dostaje lepsze rezultaty
							glm::vec3 position_j_in_neighbourhood = (neighbourhood_centre - particle_j.position);
							mass_x_position_sum += c::particleMass * position_j_in_neighbourhood;
							mass_sum += c::particleMass;

							++neighbourhood_no;

							++particle_j_ptr;
						}

					}
				}
			}

			//posumowane
			center_mass_distance = mass_x_position_sum / mass_sum;

			// if its distance to the center of mass of its neighborhood
			// is larger than a certain threshold
			if(glm::length(center_mass_distance) > c::centerMassThreshold || neighbourhood_no <= c::surfaceNeighbourhoodThreshold)
			{
				particle_i.at_surface = true;
				surface_particles.emplace_back(particle_i);
			}
			else
				particle_i.at_surface = false;

			threshold_values.push_back(glm::length(center_mass_distance));

			++particle_i_ptr;
		}
	}

	return surface_particles;
}

std::vector<Particle> Simulation::extract_surface_particles_2()
{
	auto & particles = particle_system.particles;
	std::vector<Particle> surface_particles;
	surface_particles.reserve(static_cast<unsigned int>(c::N * 0.8f));

	for(auto & particle : particles)
	{
		if(particle.at_surface == true)
			surface_particles.emplace_back(particle);
	}

	return surface_particles;
}

void Simulation::emit_particles()
{
	using namespace c;

	if(particle_count >= c::N)
		return;

	float const additional_margin = 0.5f;
	float const placement_mod = 0.4f;
	auto & particles = particle_system.particles;

	// dam break setup
	for(float x = xmin*placement_mod - 0.25f; x < xmax*placement_mod; x += c::H*additional_margin)
		for(float y = ymin*placement_mod - 0.25f; y < ymax*placement_mod; y += c::H*additional_margin)
			for(float z = zmin*placement_mod - 0.1f; z < zmax*placement_mod + 0.1f; z += c::H*additional_margin)
			{
				Particle& tp = particles[particle_count];
				tp.position = glm::vec3(x, y, z);
				tp.velocity = glm::vec3(0.0f);
				//tp.eval_velocity = glm::vec3(0.0f);
				//tp.previous_position = tp.position;
				++particle_count;
				if(particle_count >= c::N)
					return;
			}
}

void Simulation::compute_density()
{
	using particle_system::get_cell_index;
	using particle_system::out_of_grid_scope;
	using namespace c;
	const float h_sq = c::H*c::H;

	auto & grid = this->grid.grid;
	
	// go through all grids
	for(auto & i : grid)
	{
		Particle * particle_i_ptr = i.first_particle;

		// go through all particles in grid [i]
		for(int ii = 0; ii < i.no_particles; ++ii)
		{
			Particle & particle_i = *particle_i_ptr;
			particle_i.density = 0.0f;

			// go through neighbours of particle [ii] in grid [i]
			for(int z = -1; z <= 1; ++z)
			{
				for(int y = -1; y <= 1; ++y)
				{
					for(int x = -1; x <= 1; ++x)
					{
						glm::vec3 neighbour_cell_vector = particle_i.position + glm::vec3(x*c::dx, y*c::dy, z*c::dz);
						if(out_of_grid_scope(neighbour_cell_vector))
							continue;

						int neighbour_grid_idx = get_cell_index(neighbour_cell_vector);
						if(neighbour_grid_idx < 0 || neighbour_grid_idx >= c::C)
							continue;

						Particle * particle_j_ptr = grid[neighbour_grid_idx].first_particle;
						
						for(int j = 0; j < grid[neighbour_grid_idx].no_particles; ++j)
						{
							Particle& particle_j = *particle_j_ptr;

							glm::vec3 rVec = particle_i.position - particle_j.position;
							float r_sq = dot(rVec, rVec);
							float r = sqrt(r_sq);

							if(r > c::H)
							{
								++particle_j_ptr;
								continue;
							}

							particle_i.density += c::particleMass*W_poly6(r_sq, h_sq, c::H);

							++particle_j_ptr;
						}

					}
				}
			}

			// compute pressure
			particle_i.pressure = c::gasStiffness * (pow(particle_i.density / c::restDensity, 7) - 1.0f);// Tait equation
			//particle_i.pressure = c::gasStiffness * (particle_i.density - c::restDensity);

			++particle_i_ptr;
		}
	}
}

void Simulation::compute_forces()
{
	using particle_system::get_cell_index;
	using particle_system::out_of_grid_scope;
	using namespace c;
	// const float h_sq = c::H*c::H;

	auto & grid = this->grid.grid;

	// go through all grids
	for(auto& i : grid)
	{
		Particle * particle_i_ptr = i.first_particle;

		// go through all particles in grid [i]
		for(int ii = 0; ii < i.no_particles; ++ii)
		{
			Particle & particle_i = *particle_i_ptr;

			glm::vec3 totalF(0.0f);
			glm::vec3 pressureF(0.0f), viscosityF(0.0f), externalF(0.0f), surfacetensionF(0.0f);
			glm::vec3 colorFieldGrad(0.0f);
			float colorFieldLap(0.0f);

			// go through neighbours of particle [ii] in grid [i]
			for(int z = -1; z <= 1; ++z)
			{
				for(int y = -1; y <= 1; ++y)
				{
					for(int x = -1; x <= 1; ++x)
					{
						glm::vec3 neighbour_cell_vector = particle_i.position + glm::vec3(x*c::dx, y*c::dy, z*c::dz);
						if(out_of_grid_scope(neighbour_cell_vector))
							continue;

						int neighbour_grid_idx = get_cell_index(neighbour_cell_vector);
						if(neighbour_grid_idx < 0 || neighbour_grid_idx >= c::C)
							continue;

						Particle * particle_j_ptr = grid[neighbour_grid_idx].first_particle;

						for(int j = 0; j < grid[neighbour_grid_idx].no_particles; ++j)
						{
							Particle& particle_j = *particle_j_ptr;

							glm::vec3 rVec = particle_i.position - particle_j.position;
							float r = glm::length(rVec);

							if(r > c::H)
							{
								++particle_j_ptr;
								continue;
							}

							glm::vec3 gradW_poly = GradW_poly6(r, c::H)*rVec;
							colorFieldGrad += c::particleMass*gradW_poly / particle_j.density;
							colorFieldLap += c::particleMass*LapW_poly6(r, c::H) / particle_j.density;

							if(particle_i.id == particle_j.id)
							{
								++particle_j_ptr;
								continue;
							}

							//viscosityF += (particle_j.velocity - particle_i.velocity)*LapW_viscosity(r, c::H)*c::particleMass / particle_i.density;

							viscosityF += 2.0f * c::particleMass / (particle_j.density + particle_i.density) * (particle_i.velocity - particle_j.velocity) * ((rVec * Grad_BicubicSpline(rVec, c::H)) / (rVec * rVec + 0.01f*pow(c::H, 2)));

							//pressureF -= (0.5f*(particle_j.pressure + particle_i.pressure) / (particle_j.density)*c::particleMass)*GradW_spiky(r, c::H)*rVec;

							pressureF += c::particleMass*(particle_j.pressure / pow(particle_j.density, 2) + particle_i.pressure / pow(particle_i.density, 2))*GradW_spiky(r, c::H)*rVec;

							++particle_j_ptr;
						}
					}
				}
			}

			float colorFieldGradMag = glm::length(colorFieldGrad);
			if(colorFieldGradMag > c::surfaceThreshold)
				surfacetensionF = -c::surfaceTension*colorFieldLap*colorFieldGrad / colorFieldGradMag;// -sigma*nabla^{2}[c_s]*(nabla[c_s]/|nabla[c_s]|)

			if(colorFieldGradMag > c::surfaceParticleGradientThreshold)
				particle_i.at_surface = false;
			else
				particle_i.at_surface = false;

			pressureF *= -particle_i.density;
			viscosityF *= c::viscosity;// *particle_i.density;
			externalF = glm::vec3(0.0f, c::gravityAcc*particle_i.density, 0.0f);

			totalF = pressureF + viscosityF + surfacetensionF + externalF;

			particle_i.acc = totalF / particle_i.density;
			particle_i.color_field_gradient_magnitude = colorFieldGradMag;

			++particle_i_ptr;

		}
	}
}

bool save_screenshot(std::string filename, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w*h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*) malloc(nSize*sizeof(char));

	if(!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint) 0, (GLint) 0,
		(GLint) w, (GLint) h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//Now the file creation
	FILE *filePtr;
	fopen_s(&filePtr, filename.c_str(), "wb");
	if(!filePtr) return false;


	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char header[6] = { w % 256, w / 256,
		h % 256, h / 256,
		24, 0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	return true;
}

void Simulation::advance()
{
	auto static iteration_count = 0u;
	auto static sim_time = 0.0f;
	using namespace c;
	using std::chrono::high_resolution_clock;
	using std::chrono::milliseconds;
	// http://stackoverflow.com/questions/16056300/runge-kutta-rk4-not-better-than-verlet?rq=1
	glm::vec3 kinetic_force(0.0f), potential_force(0.0f);
	auto & particles = particle_system.particles;
	
	for(auto & p : particles)
	{
		// 0. semi-implicit Euler
		//glm::vec3 new_velocity = p.velocity + p.acc*dt;
		//glm::vec3 new_position = p.position + new_velocity*dt;

		// 1. velocity Verlet O(dt^3)
		glm::vec3 new_position = p.position + p.velocity*dt + 0.5f*p.acc*dt*dt;
		glm::vec3 new_velocity = (new_position - p.position) / dt;

		// 2. position Verlet O(dt^4): http://www.saylor.org/site/wp-content/uploads/2011/06/MA221-6.1.pdf
		//glm::vec3 new_position = 2.0f*p.position - p.previous_position + p.acc*dt*dt;// r_(t+dt)
		//glm::vec3 new_velocity = (new_position - p.position)/dt;// v_(t+dt)
		
		// 3. Leapfrog: http://einstein.drexel.edu/courses/Comp_Phys/Integrators/leapfrog/
		//glm::vec3 half_velocity = p.velocity + p.acc*dt; // v(t+1/2) = v(t-1/2) + a(t)*dt
		//glm::vec3 eval_velocity = (p.velocity + half_velocity)*0.5f; // v(t+1) = [v(t-1/2) + v(t+1/2)] * 0.5; used to compute forces later
		//glm::vec3 new_velocity = half_velocity;// new_velocity = v(t+1/2)
		//glm::vec3 new_position = p.position + half_velocity*dt; // p(t+1) = p(t) + v(t+1/2) dt

		//p.previous_position = p.position;
		p.position = new_position;
		//p.eval_velocity = eval_velocity;
		p.velocity = new_velocity;

		potential_force += p.position * glm::abs(p.acc);
		kinetic_force += glm::pow(new_velocity, glm::vec3(2.0f));
	}
	
	iteration_count++;
	sim_time += dt;
	mechanical_energy = 0.5f*c::particleMass*glm::length(kinetic_force) + c::particleMass*glm::length(potential_force);
	//auto d = std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - start_time);
	//if(iteration_count % 5u == 0)
	//	energy_stats.push_back(std::make_pair(sim_time, static_cast<float>(iteration_count) / static_cast<float>(d.count())));

	//	save_screenshot(std::string("./../screenshot/screen_dt_" + std::to_string(sim_time) + ".tga"), c::width, c::height);
}

void Simulation::resolve_collisions()
{
	// coefficient of restitution:
	// To cancel out the velocity in the normal direction we set cR = 0,
	// which models the normally applied no-slip condition for a liquid,
	// i.e. an inelastic collision, while models an elastic collision as in (4. 56)
	static const float cR = 0.5f;
	static const float particleBounceRadius = 0.05f;

	auto const & walls_positions = bounding_box.surface_positions;
	auto const & walls_normals = bounding_box.surface_normals;

	for(auto & tp : particle_system.particles)
	{
		for(unsigned j = 0; j < bounding_box.no_surfaces; ++j)
		{
			auto const & wall_position = walls_positions[j];
			auto const & wall_normal = walls_normals[j];


			// fluids method --------------------------------------------------
			// c::H - particle radius
			float simulation_scale = 1.0f;
			float epsilon = 0.00001f;
			float wall_particle_distance = c::H - fabs(dot(wall_position - tp.position, wall_normal)*simulation_scale);

			if(wall_particle_distance > epsilon)
			{
				float spring = c::wall_stiffness*wall_particle_distance + c::wall_damping*dot(wall_normal, tp.velocity);//eval_velocity
				tp.acc += spring*wall_normal;
			}

			// ----------------------------------------------------------------


			//float d = c::H;
			//float dist2 = fabs(dot(wall_position - tp.position, wall_normal) + particleBounceRadius);

			//// distance method:
			//// http://sccg.sk/~durikovic/publications/Pub09_11_files/SCCG2010_SPH_Flood.pdf
			//if(dist2 < d)//dist2 < d
			//{
			//	static const float c = 4.5f;

			//	// if(tp.velocity*wallNormal < glm::vec3(0.0f))
			//	tp.velocity += c*(2.0f*d - dist2)*wall_normal;
			//}

			//if(dist2 < 0.01f)// dist2 < 0.0f
			//{
				// 1.
				// tp.acc() += c::wallStiffness * wallNormal * dist2;
				// tp.acc() += c::wallDamping * dot(tp.position, wallNormal) * wallNormal;
				// 2.
				// float velNorm = glm::length(tp.velocity);//sqrt(dot(tp.velocity, tp.velocity));
				// tp.position += fabs(dist2)*wall_normal;
				// tp.velocity -= 2.0*dot(tp.velocity, wallNormal)*wallNormal;// kelager (4.56)
				// tp.velocity -= (1.0 + cR)*dot(tp.velocity, wallNormal)*wallNormal;// kelager (4.57)
				// tp.velocity -= (1.0f + cR*(fabs(dist2) / (c::dt*velNorm)))*dot(tp.velocity, wall_normal)*wall_normal;// kelager (4.58)

				// 3.
				// tp.velocity += fabs(dist2) * wallNormal / dt;

				// tp.position = tp.position - tp.velocity*dt;

				// 4.
				// r_c - miejsce kolizji
				// r_12 - obliczone poprzednie polozenie czastki (polozone 'za' przeszkoda)
				// glm::vec3 r_c = tp.position;

				// float velNorm = math::mag(tp.velocity);//sqrt(dot(tp.velocity, tp.velocity));

				// float r_rc = dot(wallNormal, r_c);
				// float r_r1 = dot(wallNormal, tp.posPrev());
				// glm::vec3 r_12 = tp.posPrev() - (1.0f + cR)*(r_r1 - r_rc)*wallNormal;

				// tp.position += wallNormal*fabs(dist2);
				// tp.posPrev() = r_12;
				// tp.velocity -= (1.0 + cR*(fabs(dist2)/(dt*velNorm)))*dot(tp.velocity, wallNormal)*wallNormal;//(1.0 + cR*(fabs(dist2)/(dt*velNorm)))

				// jesli wykrylem kolizje to licze ja tylko dla jednej sciany
				// fix zeby naprawic zrabane normalne scin w rogach
				// break;
			//}
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

	return glm::vec3(coefficient * pow((h - r), 2) / r);
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

glm::vec3 Simulation::Grad_BicubicSpline(glm::vec3 x, float h)
{
	auto const r = glm::length(x);
	auto const q = r / h;
	auto coefficient = 6.0f * (8.0f/c::PIf) / pow(h, 3);

	if(0.0f <= q && q <= 0.5f)
		coefficient *= 3.0f * pow(q, 2) - 2.0f * q;
	else if(0.5f < q && q <= 1.0f)
		coefficient *= -1.0f * pow(1.0f - q, 2);
	else
		coefficient *= 0.0f;

	return glm::vec3(coefficient) * glm::normalize(x) / h;
}
