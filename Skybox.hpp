#pragma once
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Paintable.hpp"

class Skybox : public Paintable
{
public:
	Skybox();
	~Skybox();

	void paint(Painter& p) const override final;
	void setup_buffers() override final;

	GLuint getTexture() const { return _texture; }
	static GLuint getSkyboxTexture() { return _texture; }

private:
	void loadSkyboxTextures();
	GLuint loadCubemap(std::vector<const GLchar*>& faces);

	static GLuint _texture;
	static GLfloat skyboxVertices[];
};
