#pragma once
#include "Shader.hpp"

class Box;
class BoxEditor;
class Grid;
class Skybox;
class MCMesh;
class DistanceField;
class ParticleSystem;

class Camera;

class Painter
{
public:
	Painter();

	void paint(Box const & box);
	void paint(BoxEditor const & box_editor);
	void paint(Grid const & grid);
	void paint(Skybox const & sb);
	void paint(MCMesh const & msh);
	void paint(DistanceField const & df);
	void paint(ParticleSystem const & particle);

	void set_camera(Camera const & camera) { camera_ref = &camera; }

private:
	void paint_to_framebuffer(DistanceField const & df);

	Shader bounding_box_shader;
	Shader box_editor_shader;
	Shader shader;
	Shader particle_bin_shader;
	Shader skybox_shader;
	Shader mesh_shader;
	Shader draw_normals_shader;
	Shader distance_field_raymarching;
	Shader color_cube_framebuffer_shader;

	Camera const * camera_ref;// Reference from Application class
};
