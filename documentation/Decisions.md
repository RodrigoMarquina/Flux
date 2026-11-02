# Decisions

A running log of the architectural decisions in FLUX: what was chosen, what was rejected, and why.
Newest decisions go at the bottom. A decision is only reversed by a new entry that says so — old
entries are never edited to look like they were always right.

---

## D1 — Voxel data is Structure of Arrays

*2026-10-30 · Phase 3*

Each chunk stores one contiguous array per field (`material[]`, `energy[]`, ...) instead of one
array of voxel structs.

**Why.** The CPU fetches memory in 64-byte cache lines. A simulation pass touches only a few fields.
With AoS, a 16-byte voxel where the heat pass needs 6 bytes wastes 62.5% of every line loaded. With
SoA, a line of `energy` holds 16 values, all of them used. Adding a new field to the voxel costs the
existing passes nothing, because they never load it.

SoA also removes struct padding. A struct of `uint8_t, uint32_t, uint8_t` is 12 bytes in memory
(8 if reordered) because the compiler aligns the 4-byte member to a multiple of 4. A single-type
array has no padding.

**Accepted cost.** An operation that needs every field of one voxel (a phase change) touches one
cache line per field instead of one in total. These events are rare; sweeps are constant.

**Rejected.** AoS — wins only for per-voxel operations that touch all fields, which are the rare
case here.

---

## D2 — The stored quantity is energy, not temperature

*2026-10-30 · Phase 3*

Each voxel stores its thermal energy. Temperature is derived from energy and material properties.

**Why.** During a phase change, energy goes in while temperature stays flat (latent heat). A voxel
at copper's melting point could be 0% or 99% melted; temperature alone cannot tell which, so storing
it loses information. Energy does not.

**Type: `uint32_t`, integer.** Floats have ~24 bits of precision; adding a small transfer to a large
value rounds it away, so the receiver gains less than the sender lost and energy disappears. That
breaks the conservation invariant (EnergyEconomy §9). Integers are exact.

Unsigned because thermal energy is measured from absolute zero and never goes below it. Signed
overflow is undefined behaviour; unsigned wraps, which is defined but still a bug if it happens.

**Unit.** Chosen relative to voxel size so the hottest plausible voxel uses a small fraction of the
range. Reference estimate: a 10 cm iron voxel (~7.9 kg) at 2000 K holds about 9 MJ. At 1 unit = 1 J
that is ~0.2% of `uint32_t`'s ~4.3 billion. Smaller voxels → smaller unit (1 cm voxels → 1 mJ).

**Rejected.** Storing temperature (loses latent heat). Float energy (breaks conservation).

---

## D3 — Energy transfer rule

*2026-10-30 · Phase 3*

1. Compute one integer flow per pair of neighbours.
2. Limit it to what the sender has.
3. Subtract it from one, add the same number to the other.

**Why.** Conservation is exact only if every transfer moves one integer from one place to another.
Computing each voxel's new value independently (e.g. averaging neighbours) drops division remainders
and leaks energy.

**Rejected.** Clamping at zero or at a maximum — clamping always creates or destroys energy. The
maximum is avoided by choosing the unit (D2), not by a cap.

**Known limitation.** Integer truncation means very small differences produce zero flow, so
neighbours never fully equalise. Accepted for now.

---

## D4 — Bonds are a bitmask, each voxel owns its positive directions

*2026-10-30 · Phase 3*

`connections` is a `uint8_t` bitmask. Each voxel owns only its +x, +y, +z bonds; the −x bond of a
voxel is read from its −x neighbour's +x bit.

**Why.** Adjacency is free — it is geometry, implied by the grid index. What must be stored is
whether the bond between two adjacent voxels is intact, since two voxels can touch without being
bonded (after a fracture). Storing each bond once means it has exactly one owner: no two copies to
keep in sync, and no two threads writing the same bond.

A "part" (a rock, a tool) needs no object: it is a connected group of bonded voxels, and emerges.

**Rejected.** Storing both sides (redundant, can disagree, races across chunk boundaries). Pointers
or linked structures between voxels (each hop is a cache miss).

**Open.** Bonds on a chunk's positive edge point into the neighbouring chunk — who reads and writes
them when chunks run on separate threads.

---

## D5 — Derived, not stored

*2026-10-30 · Phase 3*

| Field | Derived from |
|---|---|
| `temperature` | energy + material properties |
| `state` | energy compared to the material's melting and boiling thresholds |
| `conductivity` | material table |

**Why.** One source of truth. A stored copy of a derived value can drift out of sync with the value
it came from. The material table is tiny, so it stays in the fastest cache and deriving is cheap.
Revisit `state` only if profiling shows the derivation is hot.

---

## D6 — Velocity is sparse, stored with the active list

*2026-10-30 · Phase 3*

Almost no voxels are moving at any moment. A 12-byte velocity vector in every voxel would be almost
entirely zeros, so velocity is stored only for active voxels, alongside the active list.

When a moving voxel stops, its kinetic energy becomes thermal energy — added to `energy` — so
motion and heat are conserved together. Kinetic energy itself is never stored.

**Rejected.** A hash map from voxel to velocity — O(1) on average, but hashing plus pointer chasing
means cache misses in a hot loop.

**Open.** Looking up a voxel's velocity by position (collisions).

---

## D7 — The active list is a flat array per chunk

*2026-10-30 · Phase 3*

Each chunk keeps a contiguous array of the indices of its active voxels. A chunk with an empty list
sleeps. A voxel is added when something disturbs it (e.g. receives energy) and removed when it
settles. A 1-bit membership flag per voxel prevents duplicates; it is cleared on removal, so the
cost scales with the active count, not the world size.

**Rejected.** A single global list (active voxels scattered across memory). Node-based containers
(linked lists, maps) — good Big-O, poor cache behaviour.

**Open.** Eviction and priority policy when the list exceeds budget (EnergyEconomy §10).

---

## D8 — Voxels are 1 cm; energy unit is 10 µJ

*2026-11-01 · Phase 3*

A voxel is a 1 cm cube. Energy stays `uint32_t`, with 1 unit = 10 µJ (max ≈ 43 kJ per voxel).

**Why 1 cm.** Phase 6 needs a stone tool shaped by physics. A hand axe cannot be knapped out of
1 m voxels. The current renderer's 1 m voxels were a placeholder.

**Why 10 µJ.** The unit must cover both extremes in the same field:

| Case | Energy | Units |
|---|---|---|
| 1 cm iron at 2000 K (~7.9 g) | ~9 kJ | ~900 million — ~4.7× headroom below the `uint32_t` max |
| 1 cm air, 1 K change (~1.2 mg) | ~1.2 mJ | ~120 — ~0.01 K resolution |

At 1 mJ, a 1 K change in air would be about one unit — too coarse to diffuse smoothly. At 1 µJ, hot
iron overflows `uint32_t`.

**Accepted cost.** At 1 cm, 1 m³ is a million voxels (6 MB at 6 bytes each). This is what the active
list and sleeping chunks exist for.

**Rejected.** `uint64_t` energy — no range worries, but half as many values per cache line in the
hottest sweep. Not needed yet.

**Open.** Vaporised iron (~70 kJ per voxel) exceeds the range. Revisit when designing the planetary
core. Sums of energy across a chunk or the world are always `uint64_t`.

---

## D9 — CPU derives temperature, GPU derives colour

*2026-11-01 · Phase 3*

Each face instance carries the voxel's temperature as a `float`. The CPU converts energy to
temperature; the shader converts temperature to a glow colour.

**Why.** Glow depends on temperature, not energy — the same energy is a very different temperature
in stone than in air, because the conversion depends on the material. Converting energy in the
shader would mean a second copy of the energy → temperature rule, which would eventually disagree
with the simulation's (D5). Physics stays in one place, on the CPU. Appearance — temperature to
colour — is the same for every material (no glow below ~800 K, then dark red → orange → yellow →
white), so it belongs in the shader, and can change without touching simulation code.

`float` rather than an integer because integer values passed from the vertex to the fragment
shader must be `flat` in GLSL; a float interpolates normally.

**Rejected.** Raw energy to the GPU (duplicates the physics). Colour computed on the CPU (appearance
mixed into simulation code; any visual change means CPU changes).

---

## D10 — Fixed simulation timestep, decoupled from frame rate

*2026-11-02 · Phase 3*

The simulation advances in fixed ticks of constant `dt`. Each frame measures the real time that
passed since the previous frame, adds it to an accumulator, and runs as many ticks as that time
covers — zero, one, or several — then draws once.

**Why.** The tick, not the frame, is the simulation's unit of time. With a variable `dt` the physics
depends on frame rate: results differ between machines and runs, and one slow frame produces one
huge, possibly unstable step. With a fixed `dt` the physics is identical everywhere and its
stability (D3: explicit diffusion) is known in advance.

**Overload.** If ticks take longer to compute than the time they represent, the accumulator would
grow forever and each frame would run more ticks than the last (the "spiral of death"). Ticks per
frame are capped; time beyond the cap is dropped. The world then runs slower than real time.
That is deliberate — it is *computation is the boundary of reality* (EnergyEconomy §10) made
literal: when the world costs more than the machine can pay, time itself slows.

**Energy check** runs after every tick, not every frame, so a conservation failure names the tick
that caused it.

**Rejected.** Variable `dt` from frame time. One tick per frame (ties simulation speed to FPS).

---

## Current voxel layout

| Field | Type | Bytes |
|---|---|---|
| `material_type` | `uint8_t` | 1 |
| `energy` | `uint32_t` | 4 |
| `connections` | `uint8_t` | 1 |
| **Total** | | **6** |

## Parked

- **Electrical energy.** Current is driven by potential difference, a new kind of quantity, not more
  energy. Belongs to the Electrical Age.
