#include "Application.hpp"

Application::Application()
{
	_paintables[0] = &_sim.particle_system;
	_paintables[1] = &_sim.grid;
}

void Application::paint(void)
{
	//_grid.paint(_painter);
	//_indexer.paint(_painter);

	for(auto & prop : _paintables)
		prop->paint(_painter);
}


void Application::tick(float dt)
{
	_sim.run(dt);
}
