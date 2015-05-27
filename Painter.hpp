#pragma once
#include "Shader.hpp"

class Box;
class Grid;
class MCMesh;
class ParticleSystem;

class Camera;

class Painter
{
public:
	Painter();

	void paint(Box const & box);
	void paint(Grid const & grid);
	void paint(MCMesh const & msh);
	void paint(ParticleSystem const & particle);

	void set_camera(Camera const & camera) { camera_ref = &camera; }

private:
	Shader bounding_box_shader;
	Shader shader;
	Shader particle_bin_shader;
	Shader mesh_shader;

	Camera const * camera_ref;// Reference from Application class
};
