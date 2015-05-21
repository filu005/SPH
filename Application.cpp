#include "Application.hpp"

Application::Application()
{
	_paintables.push_back(&_sim.particle_system);
	_paintables.push_back(&_sim.bounding_box);
	//_paintables.push_back(&_sim.grid);

	_painter.set_camera(camera);
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
