#pragma once

#define SIMULATION_PAIR 0

//simulation constans
namespace c
{
	const float PIf = 3.14159265358979323846f;
	// "The larger the timestep, the smaller the smoothing kernel and the higher the stiffness,
	// the more likely the system is to explode."
	const float H = 0.03125f;//0.045
	const float gasStiffness = 4.5f;// incompressibility can only be obtained as k -> infinity.
#if SIMULATION_PAIR
	const float restDensity = 1.f;
#else
	const float restDensity = 140.f;//140
#endif
	const float particleMass = 0.00085f;//0.0008f
	const float viscosity = 0.65f;//2.5f
	const float surfaceTension = 0.55f;//0.05f
	const float surfaceThreshold = 0.00001f;
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
	const float dt = 0.004f;//0.015f
#endif
}

// grid constants
namespace c
{
	const int N = 10000;// total particle count
	const int K = 32, L = 32, M = 16;// tylko potegi 2 (bo powstaja niedokladnosci przy dzieleniu m.in. przy dx/dy/dz)
	const int C = K*L*M;
	const float xmin = -0.5f, ymin = -0.5f, zmin = -0.25f;
	const float xmax = 0.5f, ymax = 0.5f, zmax = 0.25f;
	const float dx = (xmax - xmin) / static_cast<float>(K);// tez tylko potegi 2, np. 2^(-6)=1/64
	const float dy = (ymax - ymin) / static_cast<float>(L);
	const float dz = (zmax - zmin) / static_cast<float>(M);
}

// rendering constants
namespace c
{
	auto const surfaceParticleGradientThreshold = 1.0f;// threshold for 2. surface-particle-search method
	auto const centerMassThreshold = 0.004f;
	auto const surfaceNeighbourhoodThreshold = 16u;
	auto const voxelGridDimension = 64;
	auto const xyzmaxV = c::xmax;
	auto const xyzminV = c::xmin;
	auto const voxelSize = (c::xyzmaxV - c::xyzminV) / static_cast<float>(c::voxelGridDimension);// to samo co dx, dy, dz
	auto const voxelGrid3dSize = c::voxelGridDimension * c::voxelGridDimension * c::voxelGridDimension;
	auto const boundingCubeScale = 5.0f;
	auto const boundingCubeRadius = c::voxelSize * c::boundingCubeScale;
	auto const rmax = c::voxelSize * sqrt(3.0f);//0.0221
	auto const rmin = 0.005f;
}
