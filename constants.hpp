#pragma once

//simulation constans
namespace c
{
	// "The larger the timestep, the smaller the smoothing kernel and the higher the stiffness,
	// the more likely the system is to explode."

	// kernel radius (promien odciecia)
	const float H                = 0.03125f;//def = 0.03125f
	const float gasStiffness     = 20.0f;// incompressibility can only be obtained as k -> infinity.   [N*m/kg]
	const float restDensity      = 200.0f;//115.f   [kg/m^3]
	const float particleMass     = 0.0018f;// m = ρ*(0.66*H)^3   [kg]
	const float viscosity        = 20.0f;//0.005f; def = 1.5f  // [N*s / m^2]
	const float surfaceTension   = 0.25f;
	const float interfaceTension = 0.15f;
	const float surfaceThreshold = 0.00001f;
	const float gravityAcc       = -9.80665f;

	// for collisions with container (Box)
	const float wall_stiffness = 5000.0f;// im mniejsza tym sciany bardziej 'faluja'
	const float wall_damping = -100.0f;

	// viewport dimensions
	const int width = 1024;
	const int height = 768;
	const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	const float viewHeight = 1.2f;
	const float viewWidth = aspectRatio * viewHeight;// 1.6

	// timestep (krok czasowy)
	const float dt = 0.004f;//0.015f

	const float PIf = 3.14159265358979323846f;
}

/**
 * grid constants:
 * N			init (not total! see ParticleSystem::particle_count) number of particles
 * [K, L, M]	count of bins in X, Y, Z dimensions; defines number of grid bins.
 *				set only as power of 2 (other values causes round-off errors)
 * [xmin, xmax]	dimensions of neighbour grid (in world coordinates).
 *				best to keep those min/max constants with opposite signs
 * dx, dy, dz	dimensions of single bin. only power of 2
 */
namespace c
{
	const int N = 12000; // Simulation::particle count; def = 8000
	const int K = 16, L = 16, M = 16;// tylko potegi 2 (bo powstaja niedokladnosci przy dzieleniu m.in. przy dx/dy/dz)
	const int C = K*L*M;
	const float xmin = -0.25f, ymin = -0.25f, zmin = -0.25f;
	const float xmax = 0.25f, ymax = 0.25f, zmax = 0.25f;
	//const float xmin = -0.5f, ymin = -0.5f, zmin = -0.25f;
	//const float xmax = 0.5f, ymax = 0.5f, zmax = 0.25f;
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
	auto const xyzmaxV = 0.5f;
	auto const xyzminV = -0.5f;
	auto const voxelSize = (c::xyzmaxV - c::xyzminV) / static_cast<float>(c::voxelGridDimension);// to samo co dx, dy, dz
	auto const voxelGrid3dSize = c::voxelGridDimension * c::voxelGridDimension * c::voxelGridDimension;
	auto const boundingCubeScale = 5.0f;
	auto const boundingCubeRadius = c::voxelSize * c::boundingCubeScale;
	auto const rmax = c::voxelSize * sqrt(3.0f);//0.0221
	auto const rmin = 0.002f;
}

// box editor constants
namespace c
{
	auto constexpr extrusion_step = 0.025f;
}

// diffusion constants 
namespace c
{
	//auto constexpr nutrient_diffusion_tumor = 1.0f;   // D_t
	//auto constexpr nutrient_diffusion_healthy = 0.7f * nutrient_diffusion_tumor;   // D_h
	//auto constexpr nutrient_consumption_rate_tumor = 1.0f;  // n(x,t)
	//auto constexpr nutrient_consumption_rate_healthy = 0.7f * nutrient_consumption_rate_tumor;
	//auto constexpr nutrient_threshold = 0.5f;

	auto constexpr nutrient_diffusion_tumor = 0.001f;   // D_t
	auto constexpr nutrient_diffusion_healthy = 0.1f;   // D_h
	auto constexpr nutrient_consumption_rate_tumor = 0.5f;  // n(x,t)
	auto constexpr nutrient_consumption_rate_healthy = 0.0f;
	auto constexpr nutrient_multiply_threshold = 0.9f;
	auto constexpr nutrient_die_threshold = 0.05f;
	auto constexpr nutrient_blood_vessel_concentration = 1.0f;
}

// tumor_setup 
// added blood vessels and tumor cell in the center of system 
namespace c
{
	auto const tumor_setup = true;
}