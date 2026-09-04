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
Phase 2 — Voxel Renderer                   ✓  per-face instancing · camera · input · culling
Phase 3 — Simulation Core                  ←  SoA memory · active list · gravity · connections
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
./build/Flux
```
 
Shaders are pre-compiled to SPIR-V and included in the repo. To recompile:
 
```bash
glslc shaders/cube.vert -o shaders/cube.vert.spv
glslc shaders/cube.frag -o shaders/cube.frag.spv
```
 
---

## Open Architectural Questions

**View distance & LOD.** Naively simulating and rendering every voxel at full resolution regardless of distance caps view distance hard (see: Minecraft). The direction that fits FLUX's philosophy is reduced voxel resolution at distance — fewer, larger effective voxels the further a chunk is from the player — applied to *both* simulation and rendering, not a rendering-only trick bolted on top (Nanite-style clustering doesn't fit since it assumes static geometry; Distant Horizons-style secondary LOD meshes fight the base architecture instead of being native to it). This extends the active-list/sleeping-chunk concept to resolution as well as activity. To be decided once the chunk/voxel data model is settled — likely a Phase 3+ concern.

**Cross-chunk face visibility.** Hidden-face removal currently treats any neighbour outside the current chunk as air, so every chunk renders its full outer shell even when buried inside solid neighbours. At the present 3×3×3 world that wastes one chunk's surface; at scale it dominates, since buried chunks grow as the volume while genuinely visible surface grows as the area. Fixing it requires `World` to resolve a chunk by integer grid coordinate — the same flat-index problem already solved for voxels, one level up — and then either querying the neighbouring chunk directly or maintaining a border of ghost cells. The trade-off is a lookup per boundary face against a sync step per boundary edit.

**Occlusion culling.** Frustum culling removes what is outside the view; nothing yet removes chunks hidden *behind* other chunks. Underground, that is most of them.

---
 
## Learning Methodology
 
FLUX is also a deliberate learning exercise in graphics programming from first principles. All implementation decisions are made independently — AI is used as a Socratic guide that asks questions and challenges architectural choices, never as a code generator. Every line in this repo is understood, not just working.
 
The mathematics of 3D graphics — projection geometry, the MVP pipeline, lighting models, vector operations in rendering context — are being built and derived through the engine itself, not studied in the abstract.
 
---
 
## Current State

Phases 1 and 2 complete. The full Vulkan stack is built from scratch — window, device selection, swapchain, render pass, graphics pipeline, command recording, synchronisation, depth buffer, uniform buffer for MVP transforms — with a free-flying camera and a chunked voxel world on top of it.

**World data and render data are separate.** A chunk holds every voxel it contains, densely, indexed by `i·N² + j·N + k`. Each stored voxel carries only its material — one byte. Position is never stored, because it is derivable from the index. From that dense array a *derived* render list is built: one entry per visible face, carrying a world origin and a face direction. The simulation owns the first; the GPU only ever sees the second.

**Four culling stages, each using information only available where it runs:**

| Stage | Where | When | Basis |
|---|---|---|---|
| Hidden-face removal | CPU | once per chunk change | neighbouring voxel's material |
| Frustum culling | CPU | per frame | camera vs chunk bounds |
| Back-face culling | GPU | per frame, fixed-function | triangle winding vs view |
| Depth testing | GPU | per frame | fragment depth |

The split is deliberate: neighbour visibility is camera-independent, so it is computed once and cached; back-facing-ness changes every time the camera moves, so it belongs to hardware that redoes it for free.

**Rendering is per-face, not per-voxel.** Each visible face is a single instance — six vertices, one quad. Corner positions and per-direction colours live as constants inside the vertex shader, so there is no vertex or index buffer at all; the only per-draw data is 16 bytes of instance record. For the current 27-chunk test world this submits 16,200 faces per frame instead of the 162,000 a naive per-voxel renderer would.

The result is modest to look at and demonstrates something specific: the entire path from voxel storage to rasterised pixel has been built and reasoned about deliberately, with no engine layer between the code and the GPU.
 
---
 
*FLUX is a long project. The goal is not to ship fast. The goal is to build something real, correctly, from the ground up.*
