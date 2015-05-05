#pragma once
#include <array>

#include "Simulation.hpp"
#include "Painter.hpp"


class Application
{
public:
	Application();

	// http://gamedev.stackexchange.com/questions/63912/visitor-pattern-vs-inheritance-for-rendering
	void paint();
	void tick(float dt);

private:
	Simulation _sim;
	std::array<Paintable*, 2> _paintables;
	Painter _painter;
};
