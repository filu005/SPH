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

	// const float wallStiffness = -10000.0f;// im mniejsza tym sciany bardziej 'faluja'
	// const float wallDamping = -0.9;

	const float viewHeight = 1.2f;
	const float viewWidth = (4.0f / 3.0f) * viewHeight;// 1.6

	const unsigned int wallCount = 4;

#if SIMULATION_PAIR
	const unsigned int totalParticleCount = 2;
#else
	const unsigned int totalParticleCount = 1000;
#endif

#if SIMULATION_PAIR
	const float dt = 0.0003f;//0.0004f
#else
	const float dt = 0.0005f;//0.015f
#endif
}

namespace c
{
	const int N = 100;
	const int K = 20, L = 20, M = 20;
	const int C = K*L*M;// *M;
	const float xmin = -1.0, ymin = -1.0, zmin = -1.0;
	const float xmax = 1.0, ymax = 1.0, zmax = 1.0;
	const float dx = (xmax - xmin) / static_cast<float>(K);
	const float dy = (ymax - ymin) / static_cast<float>(L);
	const float dz = (zmax - zmin) / static_cast<float>(M);
}
