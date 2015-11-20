// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Box.hpp"
#include "Grid.hpp"
#include "Skybox.hpp"
#include "MCMesh.hpp"
#include "DistanceField.hpp"
#include "ParticleSystem.hpp"
#include "Camera.hpp"
#include "Painter.hpp"

Painter::Painter() : bounding_box_shader("./shaders/bounding_box.vert", "./shaders/default.frag"),
					 shader("./shaders/default.vert", "./shaders/default.frag"),
					 particle_bin_shader("./shaders/particle_bin.vert", "./shaders/default.frag"),
					 skybox_shader("./shaders/skybox.vert", "./shaders/skybox.frag"),
					 mesh_shader("./shaders/mesh.vert", "./shaders/mesh.frag"),
					 distance_field_raymarching("./shaders/distance_field_raymarching.vert", "./shaders/distance_field_raymarching.frag"),
					 color_cube_framebuffer_shader("./shaders/color_cube_framebuffer.vert", "./shaders/color_cube_framebuffer.frag"),
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

void Painter::paint(Skybox const & sb)
{
	const auto& VAO = sb.getVAO();
	const auto& cubemapTexture = sb.getTexture();

	skybox_shader.Use();

	// Create transformations
	glm::mat4 view;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(skybox_shader.Program, "view");
	GLint projLoc = glGetUniformLocation(skybox_shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw skybox first
	//glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
	glDepthMask(GL_FALSE);

	// skybox cube
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(skybox_shader.Program, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	//glDepthFunc(GL_LESS); // Set depth function back to default
}

void Painter::paint(MCMesh const & msh)
{
	mesh_shader.Use();

	// Create transformations
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(mesh_shader.Program, "view");
	GLint modelLoc = glGetUniformLocation(mesh_shader.Program, "model");
	GLint projLoc = glGetUniformLocation(mesh_shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	auto const & VAO = msh.getVAO();

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, msh.no_vertices);
	glBindVertexArray(0);
}

void Painter::paint(DistanceField const & df)
{
	paint_to_framebuffer(df);
	distance_field_raymarching.Use();

	GLuint front_volume_texture, back_volume_texture;
	std::tie(front_volume_texture, back_volume_texture) = df.get_front_back_color_cube_textures();

	// Create transformations
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(distance_field_raymarching.Program, "view");
	GLint modelLoc = glGetUniformLocation(distance_field_raymarching.Program, "model");
	GLint projLoc = glGetUniformLocation(distance_field_raymarching.Program, "projection");
	GLint cameraPosLoc = glGetUniformLocation(distance_field_raymarching.Program, "camera_position");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(cameraPosLoc, 1, &camera.Position[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, df.get_density_texture());
	glUniform1i(glGetUniformLocation(distance_field_raymarching.Program, "density_texture"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, front_volume_texture);
	glUniform1i(glGetUniformLocation(distance_field_raymarching.Program, "front_volume_texture"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, back_volume_texture);
	glUniform1i(glGetUniformLocation(distance_field_raymarching.Program, "back_volume_texture"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::getSkyboxTexture());
	glUniform1i(glGetUniformLocation(distance_field_raymarching.Program, "skybox"), 3);

	auto const & VAO = df.getVAO();

	// Draw cube
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Painter::paint_to_framebuffer(DistanceField const & df)
{
	GLuint const front_back_volume_FBO = df.get_front_back_volume_FBO();
	GLuint front_volume_texture, back_volume_texture;
	std::tie(front_volume_texture, back_volume_texture) = df.get_front_back_color_cube_textures();

	glBindFramebuffer(GL_FRAMEBUFFER, front_back_volume_FBO);
	color_cube_framebuffer_shader.Use();

	//glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 projection;
	assert(camera_ref != nullptr);
	auto const camera = *camera_ref;

	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, c::aspectRatio, 0.1f, 1000.0f);

	// Get their uniform location
	GLint viewLoc = glGetUniformLocation(color_cube_framebuffer_shader.Program, "view");
	GLint modelLoc = glGetUniformLocation(color_cube_framebuffer_shader.Program, "model");
	GLint projLoc = glGetUniformLocation(color_cube_framebuffer_shader.Program, "projection");
	// Pass them to the shaders
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	auto const & VAO = df.getVAO();

	// Draw cube
	GLenum buffer1[] = { GL_COLOR_ATTACHMENT0, GL_NONE };
	glDrawBuffers(2, buffer1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, front_volume_texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	// Draw reverse cube - it's back faces
	GLenum buffer2[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffer2);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, back_volume_texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

