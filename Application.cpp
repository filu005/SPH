#include "Application.hpp"

Application::Application()
{
	//_paintables.push_back(&_sim.skybox);
	_paintables.push_back(&_sim.bounding_box);
	_paintables.push_back(&_sim.particle_system);
	_paintables.push_back(&box_editor);
	//_paintables.push_back(&_sim.distance_field);
	//_paintables.push_back(&_sim.mesh);
	//_paintables.push_back(&_sim.grid);
	
	_painter.set_camera(camera);
	box_editor.set_camera(camera);
	box_editor.set_bounding_box(_sim.bounding_box);
}

void Application::paint(void)
{
	for(auto & prop : _paintables)
		prop->paint(_painter);
}


void Application::tick(float dt)
{
	_sim.run(dt);
}
