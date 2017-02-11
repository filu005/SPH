# Tutorial PL
## Przechowywanie cząsteczek w pamięci
### prace:
- [Interactive SPH Simulation and Rendering on the GPU](http://maverick.inria.fr/~Prashant.Goswami/Research/Papers/SCA10_SPH.pdf)
- [SPH Neighborhood Search](http://www.escience.ku.dk/research_school/phd_courses/archive/non-rigid-modeling-and-simulation-2010/slides/copenhagen_sphImplementation.pdf)

### najważniejszy kod:

#### Grid.hpp:

- `struct GridCell`:
```c++
struct GridCell
{
    // reference to the first Particle in a sorted list
    Particle* first_particle;
    // Particles count in the list
    int no_particles;
};
```
- `std::array<GridCell, c::C> Grid::grid`

#### ParticleSystem:

- `namespace particle_system::get_cell_index(const glm::vec3 v)`:
```c++
int get_cell_index(const glm::vec3 v)
{
    using namespace c;
    return static_cast<int>(
        floor((v.x - xmin) / dx) + 
        floor((v.y - ymin) / dy) * (float) K + 
        floor((v.z - zmin) / dz) * (float) K * (float) L
    );
}
```
- `ParticleSystem::insert_sort_particles_by_indices()`

#### Simulation:
- `Simulation::bin_particles_in_grid()`
- `Simulation::run()`:
```c++
void Simulation::run(float dt)
{
    // ...
    grid.clear_grid();
    particle_system.insert_sort_particles_by_indices();
    bin_particles_in_grid();
    // ...
}
```

## Symulacja
### prace:
- podstawa: [Particle-Based Fluid Simulation for Interactive Applications](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.121.844&rep=rep1&type=pdf)
- State of the Art: [SPH Fluids in Computer Graphics – State of the Art Report](https://graphics.ethz.ch/~sobarbar/papers/Sol14/2014_EG_SPH_STAR.pdf)

### kod:

#### Simulation:
- `Simulation::run()`:
```c++
void Simulation::run(float dt)
{
    // ...
    compute_density();
    compute_forces();
    resolve_collisions();
    advance();
    // ...
}
```