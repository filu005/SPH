#pragma once
#include <array>
#include <vector>

#include "Simulation.hpp"
#include "Painter.hpp"
#include "Camera.hpp"


class Application
{
public:
	Application();

	// http://gamedev.stackexchange.com/questions/63912/visitor-pattern-vs-inheritance-for-rendering
	void paint();
	void tick(float dt);

	Camera camera;

private:
	Simulation _sim;
	std::vector<Paintable*> _paintables;
	//std::array<Paintable*, 2> _paintables;
	Painter _painter;
};
