# FLUX
 
A voxel physics simulation engine and educational game, written from scratch in **C++23** with **Vulkan** on **Arch Linux**.
 
No engine. No middleware. Every system built and understood from first principles.
 
---
 
## What FLUX Is
 
FLUX is a real-time voxel simulation where no mechanic is scripted. Every behaviour — structural collapse, heat transfer, melting, phase transitions, electrical conduction — emerges from a small set of physical rules operating on voxel state. The player does not control the simulation. They learn to work with it.
 
Each voxel stores exactly what it needs:
 
| Field | Description |
|---|---|
| `material_type` | What it is — stone, copper, water, air |
| `temperature` | Heat in kelvin, spreads to neighbours |
| `connections[]` | Bitmask of active structural bonds to 6 neighbours |
| `state` | Solid, granular, liquid, or gas |
| `conductivity` | Rate of heat and electrical transfer |
| `velocity` | Movement vector, non-zero only when active |
 
Nothing is derived at runtime that can be stored. Nothing is stored that can be derived. The voxel struct is a constraint, not a convenience.
 
The philosophical core: **energy cannot be controlled, only redirected.** This is the Tao made mechanical.

---
 
## Development Sequence
 
Nothing moves forward until the current phase is working, tested, and understood.
 
```
Phase 0 — Documentation & Planning         ✓
Phase 1 — Vulkan Engine Foundation         ✓  window · device · swapchain · geometry rendered
Phase 2 — Voxel Renderer                   ←  instanced voxel rendering · camera · input
Phase 3 — Simulation Core                      SoA memory · active list · gravity · connections
Phase 4 — Material Properties                  heat transfer · state transitions · melting · flow
Phase 5 — Player Interaction                   collision · force application · voxel response
Phase 6 — First Real Moment                    stone tool shaped by physics, falls, is picked up
Phase 7 — Content & Gameplay                   world generation · progression arc · engagement
```
 
The progression arc mirrors human civilisational history: **Stone Age → Fire & Clay → Bronze Age → Iron Age → Steam Age → Electrical Age → Computation.** Each era is unlocked not by a quest but by the simulation making the next step necessary.
 
The world is finite and spherical — computation is the boundary of reality, not a limitation to be hidden.
 
---
 
## Build
 
**Dependencies:** Vulkan SDK, GLFW, GLM, CMake 3.20+, C++23-capable compiler
 
```bash
git clone https://github.com/RodrigoMarquina/Flux.git
cd Flux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/FLUX
```
 
Shaders are pre-compiled to SPIR-V and included in the repo. To recompile:
 
```bash
glslc shaders/cube.vert -o shaders/cube.vert.spv
glslc shaders/cube.frag -o shaders/cube.frag.spv
```
 
---
 
## Learning Methodology
 
FLUX is also a deliberate learning exercise in graphics programming from first principles. All implementation decisions are made independently — AI is used as a Socratic guide that asks questions and challenges architectural choices, never as a code generator. Every line in this repo is understood, not just working.
 
The mathematics of 3D graphics — projection geometry, the MVP pipeline, lighting models, vector operations in rendering context — are being built and derived through the engine itself, not studied in the abstract.
 
---
 
## Current State
 
Phase 1 complete. The Vulkan pipeline is established: window creation, device selection, swapchain, render pass, graphics pipeline, command recording, synchronisation, vertex buffer, depth buffer, and uniform buffer for MVP transforms. A cube is rendered. The camera system is next.
 
What exists is modest. What it demonstrates is that the entire Vulkan stack has been built from scratch, module by module, with no engine layer between the code and the GPU.
 
---
 
*FLUX is a long project. The goal is not to ship fast. The goal is to build something real, correctly, from the ground up.*
