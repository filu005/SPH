#include <cstdlib>
#include <ctime>

#include "ParticleSystem.hpp"
#include "Emitters.hpp"

Emitters::Emitters() : particle_system_ref(nullptr)
{
	srand(static_cast<unsigned int>(time(nullptr)));
}

void Emitters::emit()
{
	for (auto & emitter : _emitters)
	{
		emitter.ttl -= c::dt;
		emitter.last_emission_time += c::dt;

		if (emitter.last_emission_time >= emitter.delay)
		{
			auto const mod = [&] { return emitter.emit_radius * (1.0f - 2.0f * static_cast<float>(static_cast<float>(rand()) / RAND_MAX)); };

			auto new_position = emitter.position + glm::vec3(mod(), mod(), mod());
			particle_system_ref->add_particle(Particle(new_position, emitter.emit_velocity, emitter.fluid_rest_density, emitter.fluid_viscosity, emitter.particle_mass));

			emitter.last_emission_time = 0.0f;
		}
	}

	_emitters.erase(
		std::remove_if(_emitters.begin(), _emitters.end(), [](Emitter const & e) { return e.ttl <= 0.0f; }),
		_emitters.end());
}

bool Emitters::is_any_emitter_alive() const
{
	if(_emitters.empty())
		return false;
	else
		return true;
}
