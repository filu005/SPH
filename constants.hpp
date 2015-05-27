#pragma once

#define SIMULATION_PAIR 0

namespace c
{
	const float PIf = 3.14159265358979323846f;
	// "The larger the timestep, the smaller the smoothing kernel and the higher the stiffness,
	// the more likely the system is to explode."
	const float H = 0.04f;//0.045
	const float gasStiffness = 4.5f;// incompressibility can only be obtained as k -> infinity.
#if SIMULATION_PAIR
	const float restDensity = 1.f;
#else
	const float restDensity = 120.f;
#endif
	const float particleMass = 0.002f;
	const float viscosity = 2.5f;
	const float surfaceTension = 0.05f;
	const float surfaceThreshold = 20.0f;
	const float gravityAcc = -9.80665f;

	const float wall_stiffness = 50000.0f;// im mniejsza tym sciany bardziej 'faluja'
	const float wall_damping = -100.0f;
	const int width = 800;
	const int height = 600;

	const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	const float viewHeight = 1.2f;
	const float viewWidth = aspectRatio * viewHeight;// 1.6

	const unsigned int wall_count = 6;

#if SIMULATION_PAIR
	const unsigned int totalParticleCount = 2;
#else
	const unsigned int totalParticleCount = 1000;
#endif

#if SIMULATION_PAIR
	const float dt = 0.0003f;//0.0004f
#else
	const float dt = 0.005f;//0.015f
#endif
}

namespace c
{
	const int N = 9000;// total particle count
	const int K = 64, L = 64, M = 64;// tylko potegi 2 (bo powstaja niedokladnosci przy dzieleniu m.in. przy dx/dy/dz)
	const int C = K*L*M;
	const float xmin = -0.5f, ymin = -0.5f, zmin = -0.5f;
	const float xmax = 0.5f, ymax = 0.5f, zmax = 0.5f;
	const float dx = (xmax - xmin) / static_cast<float>(K);// tez tylko potegi 2, np. 2^(-6)=1/64
	const float dy = (ymax - ymin) / static_cast<float>(L);
	const float dz = (zmax - zmin) / static_cast<float>(M);
}
