#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

class Painter;

class Paintable
{
public:
	virtual void paint(Painter& p) const = 0;
	virtual void setup_buffers() = 0;

	GLuint getVAO() const { return VAO; }

protected:
	GLuint VAO, VBO;
};
