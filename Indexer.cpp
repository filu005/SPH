#include "Painter.hpp"
#include "Indexer.hpp"

const GLfloat Indexer::vertex[2] =
{
	0.0f, 0.0f
};

Indexer::Indexer() : bin_count(c::C), particles_count(c::N)
{
	//insert_sort_by_indices();
	setup_indexer();
}

void Indexer::paint(Painter& p) const
{
	p.paint(*this);
}

void Indexer::setup_indexer(void)
{
	int index = 0;
	//for(auto const & p : particles)
	//	translations[index++] = static_cast<glm::vec2>(p.position);

	for(auto const & cell : grid)
		for(int j = 0; j < cell.no_particles; ++j)
		{
			auto particle_position = static_cast<Particle*>(cell.first_particle + j)->position;
			translations[index] = static_cast<glm::vec2>(particle_position);
			bin_idx[index] = get_cell_index(particle_position);
			++index;
		}

	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	glGenBuffers(1, &this->instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * c::N, &this->translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->bin_idx_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->bin_idx_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * c::N, &this->bin_idx[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertex), &this->vertex[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*) 0);
	//glVertexAttribDivisor(0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->bin_idx_VBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*) 0);
	glVertexAttribDivisor(1, 1);
	//By setting the attribute divisor to 1 we're effectively telling OpenGL that the vertex attribute at attribute location x is an instanced array.

	glBindBuffer(GL_ARRAY_BUFFER, this->instance_VBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*) 0);
	glVertexAttribDivisor(2, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Indexer::reset_buffers()
{
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &this->instance_VBO);
	glDeleteBuffers(1, &this->VBO);
	glDeleteVertexArrays(1, &this->VAO);

	setup_indexer();
}

void Indexer::move_particles_around(double dt)
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
		float r = RANDOM(0.002, 0.012);
		//float tt = t*RANDOM(0.7f, 1.2f);
		float x = r*cos(t);
		float y = r*sin(t);
		
		p.position = p.position + glm::vec3(x, y, 0.0f);// *static_cast<float>(dt) *100.0f;
	}
}

void Indexer::clear_grid()
{
	std::for_each(std::begin(grid), std::end(grid), [&](GridCell& c)
	{
		c = { nullptr, 0 };
	});
	//for(auto& c : grid)
	//	c = { nullptr, 0 };
}

void Indexer::insert_sort_by_indices()
{
	const size_t size = c::N;

	for(int i = 1; i < size; i++)
	{
		const Particle temp = particles[i];
		int j;

		for(j = i - 1; j >= 0 && get_cell_index(particles[j].position) > get_cell_index(temp.position); j--)
			particles[j + 1] = particles[j];

		particles[j + 1] = temp;
	}
}

void Indexer::bin_particles_in_grid()
{
	for(auto& i : particles)
	{
		int c = get_cell_index(i.position);

		if(c < 0 || c >= c::C)
			c = 0;

		if(grid[c].no_particles == 0)
			grid[c].first_particle = &i;

		++grid[c].no_particles;
	}
}

int Indexer::get_cell_index(const glm::vec3& v)
{
	using namespace c;
	return static_cast<int>(floor((v.x - xmin) / dx) + (floor((v.y - ymin) / dy)) * (float) K);// +(floor((v.z - zmin) / dz)) * (float) K * (float) L;
}

void Indexer::print_by_position_in_particles_array(std::array<Particle, c::N>& particles)
{
	for(auto& i : particles)
	{
		printf("%d, ", i.id);
	}
}

void Indexer::print_by_position_in_grid(std::array<GridCell, c::C>& grid)
{
	for(auto& i : grid)
	{
		auto p = i.first_particle;
		for(int j = 0; j < i.no_particles; ++j)
		{
			printf("%d, ", p->id);
			++p;
		}
		printf("\n");
	}
}

