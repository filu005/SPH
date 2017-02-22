#include <glm/gtc/matrix_transform.hpp>

#include "Painter.hpp"
#include "SphereModel.hpp"
#include "ParticleSystem.hpp"


namespace particle_system
{
	int get_cell_index(const glm::vec3 v)
	{
		using namespace c;
		return static_cast<int>(
			floor((v.x - xmin) / dx) + 
			floor((v.y - ymin) / dy) * (float) K + 
			floor((v.z - zmin) / dz) * (float) K * (float) L
			);
	}

	//int get_cell_index(const glm::vec3 v)
	//{
	//	return static_cast<int>(get_z_index(get_grid_coords(v)));
	//}

	inline glm::ivec3 get_grid_coords(glm::vec3 const v)
	{
		return glm::ivec3(floor((v.x - c::xmin) / c::dx), floor((v.y - c::ymin) / c::dy), floor((v.z - c::zmin) / c::dz));
	}

	glm::vec3 get_grid_coords_in_real_system(glm::vec3 const v)
	{
		return glm::vec3(floorf((v.x - c::xmin) / c::dx)*c::dx + c::xmin, floorf((v.y - c::ymin) / c::dy)*c::dy + c::ymin, floorf((v.z - c::zmin) / c::dz)*c::dz + c::zmin);
	}

	bool out_of_grid_scope(const glm::vec3 v)
	{
		using namespace c;
		return v.x < xmin || v.x > xmax || v.y < ymin || v.y > ymax || v.z < zmin || v.z > zmax;
	}

	inline uint64_t get_z_index(glm::ivec3 const v)
	{
		return mortonEncode_magicbits(v.x, v.y, v.z);
	}

	// http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
	inline uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z)
	{
		uint64_t answer = 0;
		answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
		return answer;
	}

	uint64_t splitBy3(unsigned int a)
	{
		uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
		x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
		x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
		x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
		x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
		x = (x | x << 2) & 0x1249249249249249;
		return x;
	}
}

ParticleSystem::ParticleSystem()
{
	particles.resize(c::N);
	model_matrices.resize(c::N);
	bin_idx.resize(c::N);
	surface_particles.resize(c::N);

	setup_buffers();
}

void ParticleSystem::paint(Painter& p) const
{
	p.paint(*this);
}

void ParticleSystem::setup_buffers(void)
{
	using particle_system::get_cell_index;

	int index = 0;
	for(auto const & p : particles)
	{
		auto particle_position = p.position;
		glm::mat4 model;
		model = glm::translate(model, particle_position);
		model = glm::scale(model, glm::vec3(0.02f));
		model_matrices[index] = model;
		bin_idx[index] = static_cast<float>(get_cell_index(particle_position));
		surface_particles[index] = p.at_surface;
		index++;
	}

	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	glGenBuffers(1, &this->model_mat_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->model_mat_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * particle_count, &this->model_matrices[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->bin_idx_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->bin_idx_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * particle_count, &this->bin_idx[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->at_surface_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->at_surface_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * particle_count, &this->surface_particles[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->sphere_model.vertices), &this->sphere_model.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//point_vertices
	//glGenBuffers(1, &this->VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(this->point_vertices), &this->point_vertices[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glGenBuffers(1, &this->EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->sphere_model.indices), &this->sphere_model.indices[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	//glVertexAttribDivisor(0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->bin_idx_VBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glVertexAttribDivisor(1, 1);
	//By setting the attribute divisor to 1 we're effectively telling OpenGL that the vertex attribute at attribute location x is an instanced array.

	glBindBuffer(GL_ARRAY_BUFFER, this->at_surface_VBO);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*) 0);
	glVertexAttribDivisor(6, 1);

	glBindBuffer(GL_ARRAY_BUFFER, this->model_mat_VBO);
	// Set attribute pointers for matrix (4 x vec4)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*) 0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*) (sizeof(glm::vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*) (2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*) (3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleSystem::reset_buffers()
{
	glDisableVertexAttribArray(5);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	//glDeleteBuffers(1, &this->EBO);
	glDeleteBuffers(1, &this->VBO); 
	glDeleteBuffers(1, &this->at_surface_VBO);
	glDeleteBuffers(1, &this->bin_idx_VBO);
	glDeleteBuffers(1, &this->model_mat_VBO);
	glDeleteVertexArrays(1, &this->VAO);

	setup_buffers();
}

void ParticleSystem::update_buffers()
{
	using particle_system::get_cell_index;

	int index = 0;
	for(auto const & p : particles)
	{
		auto const particle_position = p.position;
		glm::mat4 model;
		model = glm::translate(model, particle_position);
		model = glm::scale(model, glm::vec3(0.02f));
		model_matrices[index] = model;
		bin_idx[index] = static_cast<float>(get_cell_index(particle_position));
		surface_particles[index] = p.at_surface;
		index++;
	}

	// alternatywa: http://www.gamedev.net/topic/666461-map-buffer-range-super-slow/

	glBindBuffer(GL_ARRAY_BUFFER, this->model_mat_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * particle_count, &this->model_matrices[0]);// replace data in VBO with new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->bin_idx_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * particle_count, &this->bin_idx[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->at_surface_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLuint) * particle_count, &this->surface_particles[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::unique_ptr<glm::vec4[]> ParticleSystem::get_position_color_field_data()
{
	auto position_color_field_data = make_unique<glm::vec4[]>(particle_count);
	for(int i = 0; i < particle_count; ++i)
		position_color_field_data[i] = glm::vec4(particles[i].position, particles[i].color_field_gradient_magnitude);
	return position_color_field_data;
}

void ParticleSystem::add_particle(glm::vec3 const position, glm::vec3 const velocity)
{
	particles.push_back(Particle(position, velocity));
	model_matrices.push_back(glm::mat4());
	bin_idx.push_back(0.0f);
	surface_particles.push_back(0u);

	++particle_count;

	// resize buffers
	setup_buffers();
}

void ParticleSystem::move_particles_around(float dt)
{
	//RANDOM(-0.001f, 0.001f);
	double v0 = 10.0;
	float a = -0.008f;
	static float t = 0.0f;
	t += dt*200.f;
	//float x = (a*sqrt(2)*cos(t)) / (pow(sin(t), 2) + 1);
	//float y = (a*sqrt(2)*cos(t)*sin(t)) / (pow(sin(t), 2) + 1);

	for(auto& p : particles)
	{
		float r = RANDOM(0.002f, 0.012f);
		//float tt = t*RANDOM(0.7f, 1.2f);
		float x = r*cos(t);
		float y = r*sin(t);
		float z = 5.0f*r*cos(t*10.0f)*sin(t*10.0f);

		p.position = p.position + glm::vec3(x, y, z);// *static_cast<float>(dt) *100.0f;
	}
}

void ParticleSystem::insert_sort_particles_by_indices()
{
	using particle_system::get_cell_index;

	for(int i = 1; i < particle_count; i++)
	{
		auto const particle_i = particles[i];
		auto const particle_i_idx = get_cell_index(particle_i.position);
		int j;

		for(j = i - 1; j >= 0 && get_cell_index(particles[j].position) > particle_i_idx; j--)
			particles[j + 1] = particles[j];

		particles[j + 1] = particle_i;
	}

	// http://codereview.stackexchange.com/questions/110793/insertion-sort-in-c
	//for(auto it = begin(particles) + 1; it != end(particles); ++it)
	//{
	//	auto const particle_i_idx = get_cell_index(it->position);
	//	auto insertion = it - 1;
	//	for(; insertion >= std::begin(particles) && get_cell_index(insertion->position) > particle_i_idx; insertion--);
	//	std::rotate(insertion++, it, std::next(it));
	//}
}

GLfloat const ParticleSystem::point_vertices[3] =
{
	0.0f, 0.0f, 0.0f
};
