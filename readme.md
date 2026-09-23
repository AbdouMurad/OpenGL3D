# OpenGL3D Engine — Overview and Setup

This repository contains a  3D engine implemented with OpenGL and a component system. It includes a renderer, an asset manager, scene serialization, and a simple physics system. The instructions below describe how to build and run the engine and identify key locations in the codebase.

Prerequisites

- Visual Studio 2022 with C++ workload
- CMake 3.25 or later
- vcpkg (optional) for dependency management

Build and run

1. If you use vcpkg, install and bootstrap it and integrate with Visual Studio.
2. Open the project folder in Visual Studio: File -> Open -> Folder.
3. Choose the `x64` configuration and build the project (Ctrl+Shift+B).
4. Run the application with F5 or Ctrl+F5.

If CMake configuration fails, delete the build output folder and reopen the project so CMake can reconfigure.

Repository layout

- `src/` — engine implementation and demo games
  - `Core/` — engine loop, scene management, serialization, timing
  - `Components/` — transform, camera, mesh renderer, lights, game object definitions
  - `Physics/` — `PhysicsWorld`, collision detection and response, shapes, rigid bodies
  - `Render/` — renderer, debug renderer, GPU utilities, shaders
  - `Events/` — event bus and event types
- `assets/` — shaders, models, textures, example level JSON files

Important systems

- Scene
  - `Scene` holds `GameObject` instances and runs their update methods. `Scene::Start()` gathers physics bodies and colliders.

- Physics
  - Broad phase: constructs AABBs per collider and generates candidate pairs.
  - Narrow phase: per-shape collision tests. Current supported shapes: box and sphere. Box-box uses SAT with manifold generation.
  - Solver: sequential impulse per contact, with positional correction. Solver parameters are in `PhysicsWorld.cpp`: `solverIterations`, positional correction `percent` and `slop`. Tuning these affects stack stability.

- Rendering
  - Forward renderer submitting `RenderFrame` with lights and draw calls.
  - PBR shader provided in `assets/shaders/pbr.frag` and a default shader in `assets/shaders/default.frag`.
  - `DebugRenderer` draws wireframes for colliders and debug visuals.

- Assets and serialization
  - `AssetManager` loads models and shaders.
  - `SceneSerializer` loads scene data from JSON files under `assets/level`.

Debugging and tuning

- Use the debug renderer to draw colliders and contact points.
- Common adjustments for physics stability:
  - Increase `solverIterations` to improve contact convergence for stacks.
  - Lower positional correction `percent` and increase `slop` to avoid tiny corrective pushes.
  - Add small linear damping and sleeping thresholds in `Integrate` to remove micro velocities.
  - For the most stable stacks implement persistent contact caching and warm-starting.

Where to change things

- `Core/Scene.cpp` and `Core/Scene.h` — scene lifecycle and where physics is run
- `Physics/PhysicsWorld.cpp` — integration, collision detection, and solver
- `Render/Renderer.cpp` and `Render/` — rendering and shader use
- `Components/` — add or modify components (camera, light, mesh, rigid body, collider)

Extending the engine

- Add new components by deriving from `Component` and attaching them to `GameObject` instances.
- Add new collision shapes: extend `Shape.h`, add a broadphase AABB function and a narrowphase test, then register in `Collision::init()`.
- Modify or add shaders in `assets/shaders/` and update the renderer to set uniforms as needed.

Contributing

Follow the existing coding patterns. Keep changes minimal and run the project to verify there are no regressions. If making physics changes, test stacks and resting contacts and tune solver parameters accordingly.

License

Check for a license file in the repository root before reuse.
