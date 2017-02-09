# Fluid simulator Smoothed Particle Hydrodynamics
An implementation of SPH fluid simulator.

## Dependencies
- [GLM](http://glm.g-truc.net/0.9.8/index.html)
- [GLEW](http://glew.sourceforge.net/)
- [GLFW](http://www.glfw.org/)
- [SOIL](http://www.lonesock.net/soil.html)

Code compiles with Visual Studio 2013/2015; proper project file is in this repository (z-index sort.vcxproj).
I've never tried to run it on linux or compile it with GCC (but I suspect that there wouldn't be any major problem for it to work).

## Introduction to the code
All simulating takes place in `Simulation.cpp`, in `Simulation::run(float dt)` method. Briefly:

```c++
// simulation loop
void Simulation::run(float dt)
{
    // init
    emit_particles();

    // cache optimization
    grid.clear_grid();
    particle_system.insert_sort_particles_by_indices();
    bin_particles_in_grid();

    // simulation <- all SPH stuff is here
    compute_density();
    compute_forces();
    resolve_collisions();
    advance();
    
    // rendering
    distance_field.generate_field_from_surface_particles(extract_surface_particles());
    particle_system.update_buffers();
}
```
In main simulation loop also appear methods responsible for placing every particle into correct cube in grid (ParticleSystem class; see 'z-index neighbour search' in reference 2.)
and for rendering (DistanceField class; see 'Distance Field Generation' and 'GPU Raycasting' also in reference 2.).

Painting objects (i.e. calling drawing functions) is constructed via a Visitor pattern. Every Object which is to be drawn, derives from `Drawable` class and it's reference is held in `Application::_paintables` vector.
Then in every iteration a `Application::paint()` method is called which calls `paint()` method for every Object in that `_paintables` vector, which in turn (through the Object) calls `Painter::paint(Object &)` method.

## References
1. Matthias Müller, David Charypar, and Markus Gross.
“Particle-Based Fluid Simulation for Interactive Applications”

2. Prashant Goswami et al.
“Interactive SPH Simulation and Rendering on the GPU”

3. Micky Kelager.
“Lagrangian Fluid Dynamics Using Smoothed Particle Hydrodynamics”

4. Markus Ihmsen et al.
“SPH Fluids in Computer Graphics – State of the Art Report”