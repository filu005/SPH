// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#include "Grid.hpp"
#include "ParticleSystem.hpp"
#include "Painter.hpp"

Painter::Painter() : shader{ Shader("./shaders/default.vert", "./shaders/default.frag") },
					 particle_bin_shader{ Shader("./shaders/particle_bin.vert", "./shaders/default.frag") }
{

}

void Painter::paint(Grid const & grid)
{
	auto const & VAO = grid.getVAO();
	
	shader.Use();
	glUniform1f(glGetUniformLocation(shader.Program, "bin_count"), static_cast<GLfloat>(grid.bin_count));

	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_LINE_LOOP, 0, 4, grid.bin_count);
	glBindVertexArray(0);
}

void Painter::paint(ParticleSystem const & ps)
{
	auto const & VAO = ps.getVAO();

	particle_bin_shader.Use();
	glUniform1f(glGetUniformLocation(particle_bin_shader.Program, "bin_count"), static_cast<GLfloat>(ps.bin_count));

	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 240, ps.particle_count);
	//glDrawElementsInstanced(GL_TRIANGLES, 192, GL_UNSIGNED_INT, 0, ps.particle_count);
	glBindVertexArray(0);
}
