#pragma once
#include "Shader.hpp"

class Grid;
class ParticleSystem;

class Painter
{
public:
	Painter();

	void paint(Grid const & grid);
	void paint(ParticleSystem const & particle);

private:
	Shader shader;
	Shader particle_bin_shader;
};
