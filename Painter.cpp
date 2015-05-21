// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Box.hpp"
#include "Grid.hpp"
#include "ParticleSystem.hpp"
#include "Camera.hpp"
#include "Painter.hpp"

Painter::Painter() : bounding_box_shader("./shaders/bounding_box.vert", "./shaders/default.frag"),
					 shader{ Shader("./shaders/default.vert", "./shaders/default.frag") },
					 particle_bin_shader{ Shader("./shaders/particle_bin.vert", "./shaders/default.frag") },
					 camera_ref(nullptr)
{

}

void Painter::paint(Box const & box)
{
	bounding_box_shader.Use();

	// Create transformations
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	model = glm::translate(model, box.position);
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(bounding_box_shader.Program, "view");
	GLint modelLoc = glGetUniformLocation(bounding_box_shader.Program, "model");
	GLint projLoc = glGetUniformLocation(bounding_box_shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	auto const & VAO = box.getVAO();

	glBindVertexArray(VAO);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*) (4 * sizeof(GLuint)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*) (8 * sizeof(GLuint)));
	glBindVertexArray(0);
}

void Painter::paint(Grid const & grid)
{	
	shader.Use();
	glUniform1f(glGetUniformLocation(shader.Program, "bin_count"), static_cast<GLfloat>(grid.bin_count));

	// Create transformations
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	auto const & VAO = grid.getVAO();

	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0, grid.bin_count);
	glDrawElementsInstanced(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*) (4 * sizeof(GLuint)), grid.bin_count);
	glDrawElementsInstanced(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*) (8 * sizeof(GLuint)), grid.bin_count);
	glBindVertexArray(0);
}

void Painter::paint(ParticleSystem const & ps)
{
	particle_bin_shader.Use();
	glUniform1f(glGetUniformLocation(particle_bin_shader.Program, "bin_count"), static_cast<GLfloat>(ps.bin_count));

	// Create transformations
	glm::mat4 view;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(particle_bin_shader.Program, "view");
	GLint projLoc = glGetUniformLocation(particle_bin_shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	auto const & VAO = ps.getVAO();

	glBindVertexArray(VAO);
	//glDrawArraysInstanced(GL_POINTS, 0, 1, ps.particle_count);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 240, ps.particle_count);
	//glDrawElementsInstanced(GL_TRIANGLES, 192, GL_UNSIGNED_INT, 0, ps.particle_count);
	glBindVertexArray(0);
}

