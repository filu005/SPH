#pragma once
#include <array>
#include <vector>

#include "Simulation.hpp"
#include "Painter.hpp"
#include "BoxEditor.hpp"
#include "Camera.hpp"


class Application
{
public:
	Application();
	// http://gamedev.stackexchange.com/questions/63912/visitor-pattern-vs-inheritance-for-rendering
	void paint();
	void tick(float dt);

	ParticleSystem & get_particle_system() { return _sim.particle_system; }

	BoxEditor box_editor;
	Camera camera;

private:
	Simulation _sim;
	std::vector<Paintable*> _paintables;

	Painter _painter;
};
