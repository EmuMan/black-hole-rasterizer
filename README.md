# Black Hole Rasterizer

## Instructions

To run the project, `glm` and `glfw3` are required. You can install them via your package
manager if they are available, or you can find them [here (glm)](https://github.com/g-truc/glm)
and [here (glfw)](https://www.glfw.org/download). Once those dependencies are installed, you
can build and run the project with something equivalent to the following on your platform:

```bash
mkdir build
cd build
cmake ..
make
./BlackHoleRasterizer
```

## Details

The main attraction is the black hole in the middle of the scene, which can be toggled with
the `b` key. There are a few "planets" orbiting this black hole (two of which are point lights),
and a claw (heirarchically modeled) at the bottom that is keeping it alive. The physics don't
really line up but try not to think about it too much.

You can fly around the scene with WASD + E for up and Q for down, or the arrow keys. There are
colliders for the floor, edge of the island, center construction, and pillars.

Pressing `z` reveals the wireframe structure of the models, and by pressing `f` you can enter
(and exit) freecam mode, exploring the distorted world the vertex shader creates as if you
were independent of the actual viewpoint.

## Structure

The `.obj` files are loaded in as `Mesh` objects, which are then assigned to `Object` objects
along with transform information. A mesh's transformation matrix is computed recursively,
accounting for parent transformations as well as the mesh's own. That way, when the mesh is
rendered, its transformation matrix is accurate to where it is in world space.

There are three special children of the `Object` class, which are `MeshObject`, `CameraObject`,
and `PointLightObject`. Information about each `PointLightObject` gets passed into the shaders
to do lighting calculations. `MeshObjects` can hold a material, which holds information about
how an object looks as well as an index reference to the shading program used to render that
material. This was designed to avoid duplication of shading programs and textures, and seems
to work relatively well. There are four `Material` subclasses, each designed to perform a
different task and use a different fragment shader (although each uses the same vertex
shader). The `CameraObject` contains all the necessary information to form the view and projection
matrices for the scene.

There are methods within the application for initializing all of these components.

## The Black Hole

Many of the methods used are derived from [this paper](https://www.semanticscholar.org/paper/Implementing-a-Rasterization-Framework-for-a-Black-Yamashita/90a9b04b7153462da9d8edecdfa8262bdd689a4c).

The black hole effect is derived from the Schwarzschild metric, a solution to the Einstein field
equations that can be used to accurately model light paths within black hole spacetime. As the
paper suggests, the boundary value problem (BVP) of the set of derived differential geodesic
equations can be solved to evaluate the trajectory a light ray would take between a vertex and
an observer around a black hole. Unfortunately, this is a very expensive computation, even moreso
because I was using Python's `scipy` library to brute force solutions from boundary conditions and
initial states.

My solution to this problem was similar to the paper's, albeit less... thorough. I precomputed a
set of information inside a 3D texture that could be efficiently sampled from within the vertex
shader. This information could then be used to reconstruct the scene as it would appear with the
added complexity of the black hole. Two draw calls were also used for this: one for the more
direct path light would take around the black hole, and one for the more roundabout one.

All of the scripts and baked information can be found in `resources/blackhole`.

## Shortcomings

My simulation of the black hole is not entirely accurate, especially where the transition between
primary and secondary rays occurs. This is partially because of the nature of rasterization;
straight lines between everything becomes an issue when space is heavily warped. The paper
addresses this by dynamically tessellating the mesh for more demanding areas. I addressed it by
slamming my tri count through the roof for every mesh I could get my hands on. Not a great
solution, but it still runs fine given the low computation cost otherwise.

The other issue with the simulation is with closed objects like the skybox. Because the vertex
shader is only warping existing vertices, the fully enclosed nature of the skybox causes issues
when different parts of it need to be seen. It's a little hard to explain, but it's been one of
my bigger headaches throughout this project. It's there on more standard meshes as well, but less
apparent. I ended up using a hacky workaround and fine tuning some parameters that shouldn't even
exist, so I'd like to revisit it with a couple of other ideas I have to see if I can get a more
elegant solution working.
