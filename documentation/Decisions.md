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

## D11 — Heat crosses chunk boundaries; ticks are two-phase at world level

*2026-11-02 · Phase 3*

D4's ownership rule extends across chunks: a voxel on a chunk's positive edge owns the pair with
the voxel at coordinate 0 of the next chunk in that direction. Each chunk keeps a persistent delta
array. A tick is: compute pass over **all** chunks, then apply pass over **all** chunks.

**Why.** Chunks are a memory layout, not a physical boundary. Skipping cross-chunk pairs made every
chunk a perfect insulator. Two phases at world level are required for the same reason as within a
chunk (D3): if one chunk applied its changes before the next computed, boundary flows would read
already-updated energies and heat would drift in the order chunks are processed.

Neighbour chunks are found by index, never by pointer — a pointer into a `std::vector` dangles
when the vector reallocates.

**Accepted cost.** A chunk's compute pass writes into a neighbour's delta array. Safe while
single-threaded; a data race once chunks run on separate threads.

**Open.** Threading the compute pass (see D4). Halo/ghost layers are the likely answer.

---

## D12 — Per-tick temperature cache

*2026-11-03 · Phase 3*

Each chunk keeps a `float temperatures[]` array, filled from energy at the start of every tick. A
tick is now three phases over all chunks: update temperatures → compute flows → apply flows.

**Why.** Each voxel's temperature was derived up to four times per tick (once as itself, up to three
times as a neighbour), each a division. Measured in a Release build, 27 000 voxels, averaged over
~16 s:

| | Temperatures + compute per tick |
|---|---|
| Before | ~0.34 ms |
| After | ~0.27 ms (~20% faster) |

Printed temperatures were identical to the uncached run; energy total unchanged.

**Not a violation of D5.** The cache is scratch data, like the delta array: rebuilt from energy every
tick and never written back. Energy remains the only source of truth. D5 forbids a *persistent*
derived copy that can drift.

**`float`, not `double`.** A `double` cache (8 bytes) would be larger than the voxel itself (6 bytes)
and double the memory the tick reads. Float precision at 300 K is ~0.00003 K — far finer than any
flow needs — and conservation is unaffected because flows remain integers.

`fillFaceRenderList` still derives temperature directly: it runs after the apply phase, when the
cache is one tick stale.

**Context.** At 1 cm voxels, 1 m³ is ~10⁶ voxels → ~10 ms per tick at this rate, most of a 16.7 ms
budget for heat alone. Per-voxel speedups like this are worth tens of percent; the active list is
worth orders of magnitude.

---

## D13 — Voxels sleep below a temperature threshold; no snapping to equilibrium

*2026-11-03 · Phase 3*

A voxel stays on the active list only while the temperature difference with at least one neighbour
is above a sleep threshold (provisionally 0.1 K). Below it, the voxel is not computed. Any small
differences that remain stay frozen in place, and the voxel resumes from them if something
disturbs it again.

**Why.** Diffusion has a long tail. For stone, α = k / (ρc) ≈ 9 × 10⁻⁷ m²/s, and the time to even
out a distance L is ~L²/α: ~2 minutes across 1 cm, ~3 hours across a 10 cm chunk. If "active" meant
"any heat still moving", a warm region would cost compute for hours, mostly on differences no player
can see. A threshold makes "only simulate what matters" a measurable rule. Sleeping moves no energy,
so conservation holds for free.

D3's integer truncation already stops flow below ~0.03 K for stone at the current `dt`. The
threshold makes that cutoff a deliberate choice instead of a rounding accident.

**Rejected.** Snapping a settled region to its average temperature. Energy can be redistributed
exactly, but heat would cross the whole region in one tick, faster than conduction allows. A large
snapped region becomes a heat teleporter.

**Open.** The threshold value (and whether it depends on the material). How voxels wake up: see D14
and D15.

---

## D14 — Active list mechanics: two lists, two flags, six directions

*2026-11-03 · Phase 3*

Each chunk keeps `active` (indices computed this tick) and `nextActive` (indices to compute next
tick), each with its own per-voxel membership flag. The compute pass iterates only `active`:

- **+x, +y, +z:** computed as before (D4: the voxel owns these pairs).
- **−x, −y, −z:** computed only if the neighbour is not in `active`. An active neighbour computes
  the pair itself; a sleeping one never would. Every pair is still computed exactly once.
- **Pair ΔT below the threshold (D13):** skipped entirely — no transfer.
- **Otherwise:** flow computed, both voxels added to `nextActive` (in their own chunks).

Apply iterates only `nextActive`: since sub-threshold pairs transfer nothing, every voxel with a
non-zero delta is in it. After apply, the lists and flags swap and the new `nextActive` is cleared
by iterating it (cost scales with the active count, D7).

**Why two lists.** Appending to a `std::vector` while iterating it can reallocate it, leaving the
loop's reference pointing at freed memory — undefined behaviour.

**Why two flags.** A voxel can be in `active` but not yet in `nextActive`; one flag cannot hold
both answers.

**Why not in apply.** Flows must all be computed from one snapshot of temperatures (D11).

**Accepted cost.** Negative directions must cross into the previous chunk, adding a second boundary
case. The neighbour's `active` flag is read in the hot loop.

---

## D15 — Changing a voxel's energy wakes it; the temperature cache refreshes only active voxels

*2026-11-04 · Phase 3*

Revises D12, which rebuilt the whole cache every tick.

- **At chunk creation:** every voxel's cached temperature is computed once, from its energy.
- **Each tick:** only the voxels in `active` are refreshed. After the swap, `active` is exactly the
  set whose energy changed in the previous tick's apply; every other voxel's energy, and therefore its
  cached temperature, is unchanged.
- **Rule:** anything that changes a voxel's energy outside the tick (the hot spot today; player
  tools and fire later) must also wake it. Otherwise its cached temperature goes stale.

Both paths call one helper, so the energy → temperature rule exists in exactly one place.

**Why the full fill is required.** `resize` fills the cache with 0.0 K. Without the fill, sleeping
voxels read as 0 K: their neighbours push heat into them, wake them, and the error spreads outward as
a wave. Energy stays conserved throughout. Only the reference temperatures catch it (observed: hot
voxel 968.275 K instead of 968.503 K after 1 s, 324 active voxels instead of 19).

**Measured** — Release, 27 000 voxels, one hot voxel at 1000 K, ~17 s:

| | Per tick |
|---|---|
| D12 (every voxel, every tick) | ~0.27 ms |
| Active list (19 → 85 active voxels) | ~0.003 → 0.007 ms (~40–90× faster) |

Every printed temperature over 15 s matched the all-voxel run (hot 968.503 → 666.944 K, neighbour
307.112 → 351.317 K), and the energy total did not change. Cost now grows with the number of active
voxels, not with the size of the world.

---

## D16 — No global world state; warnings are errors in spirit

*2026-11-04 · Phase 3*

**The world is owned by `main`.** `generateWorld()` returns the chunk vector, and every function that
reads or changes the world receives it as a parameter. There is no `extern` world.

**Why.** A global can be modified from any file that includes its header, and no function signature
shows it. Once the simulation is threaded, every hidden global is a potential data race. Returning
the vector costs nothing: the compiler constructs it directly in the caller (copy elision) or moves
it. Chunks are moved into the vector with `std::move`, after a `reserve`, so their arrays are never
copied.

**Zero-warning build.** The `Flux` target compiles with `-Wall -Wextra -Wshadow`. The build must stay
warning-free: once a few warnings are tolerated, new ones go unnoticed. The first run found a
shadowed global, an ignored `VkResult`, an unused parameter, and three signed/unsigned comparisons.
`-Wconversion` was added on 2026-11-05 and is part of the same rule. Its one real finding: plain
`cos`/`sin` were the C `double` versions, widening every `float` argument and narrowing the result.

**Rejected.** Renaming the shadowing parameter (silences the warning, keeps the global). Filling a
vector passed by reference (a second call silently appends to the existing world).

---

## D17 — Accept frozen temperature hills from the sleep threshold, for now

*2026-11-05 · Phase 3*

The sleep threshold (D13) limits the temperature difference between neighbours, not across a region.
A smooth slope of under 0.1 K per voxel can therefore freeze in place until something disturbs it.

**Measured** — Release, 27 000 voxels, one voxel starting at 1000 K, 30× time scale:

| | Result |
|---|---|
| Peak active voxels | ~1 440 at ~850–910 s simulated (~5% of the world) |
| All voxels asleep | between 1 605 s and 1 634 s simulated |
| Hot voxel at rest | 302.197 K, ~1.25 K above the world average of ~300.95 K |
| Tick cost at peak / at rest | ~0.06 ms / ~0.00024 ms (all-voxel sweep: ~0.27 ms) |
| Total energy | unchanged in every print |

**Why accept it.** Energy stays exact, the difference is invisible, and any disturbance wakes the
region and lets it continue settling. Physics only has to be reliable and believable.

**Known limitation.** A frozen hill can rise by up to 0.1 K per voxel, so a 50-voxel slope could hold
about 5 K. That could matter once melting exists: a voxel could freeze a few kelvin below its melting
point. Revisit in Phase 4.

**Accepted cost.** An active voxel costs roughly 40 ns per tick against ~10 ns per voxel in the plain
sweep (six directions, flag checks, coordinate divisions, out-of-order memory access). On this
estimate, the active list stops paying off at about 25% of the world awake.

---

## D18 — The active list stays: measured break-even is ~32% of the world awake

*2026-11-05 · Phase 3*

Replaces D17's 25% estimate with a measurement. Release, 27 000 voxels, one full chunk starting at
1000 K, 30× time scale. Tick cost covers the temperature and compute phases, the same phases as the
~0.27 ms all-voxel sweep (D12).

| Active voxels | Share of world | Tick cost | Per active voxel |
|---|---|---|---|
| 3 048 | 11% | 0.078 ms | ~26 ns |
| 6 826 | 25% | 0.211 ms | ~31 ns |
| ~8 500 | ~32% | ~0.27 ms | ~32 ns |
| 14 554 | 54% | 0.505 ms | ~35 ns |
| 17 250 | 64% | 0.623 ms | ~36 ns |

Total energy held at 1 906 000 000 000 in every print.

**Decision.** Keep the active list and continue to gravity. Cost grew steadily with the active count,
with no sudden jumps. Above ~32% awake the list loses to a plain sweep, and it would cost about 1 ms
with every voxel awake (extrapolated, not reached in this run). Such events should be brief; a
mostly sleeping world is the normal case.

**Not covered by this number.** The timer excludes apply and the rest of the loop. Two costs still
scale with the world, not with activity: `getChunkEnergy` scans every voxel every tick (D10), and
`fillFaceRenderList` rebuilds every chunk every frame. At 1 m³ these would dominate, so the
active list alone does not make a large world cheap.

**FPS.** It fell from ~1 400 to ~270 in this run, mostly because of the 30× time scale: ~1 350 ticks
per second × ~0.6 ms ≈ 0.84 s of timed work per real second, and simulated time slowed to ~22 s per
print (D10). At 1× the same load is about 37 ms per second.

---

## D19 — Granular is not a state

*2026-11-05 · Phase 3*

States are solid, liquid and gas, all derived from energy (D5). "Granular" is not a state: it is solid
material with few or no bonds. A rock is many bonded voxels, gravel is small bonded clusters, and a
sand voxel has no bonds at all.

**Why.** Looseness is already described by `connections`. A separate state would be a second source
of truth that could disagree with the bonds. It also could not be derived from energy like the other
states.

**Limit.** Real sand grains are far smaller than a 1 cm voxel, so a sand voxel stands for a bulk of
grains, and cluster size can only describe particles down to one voxel.

**Open.** Unbonded voxels must slide to form piles rather than stack in columns.

---

## D20 — Bonded voxels move as one piece

*2026-11-05 · Phase 3*

A group of bonded voxels moves as a single piece. It does not fall voxel by voxel.

**Why.** Phase 6 needs a stone tool that falls, stays whole and can be picked up. Voxels moving
independently would pull a bonded rock apart.

**Accepted cost.** The voxels of a piece must be found by following its bonds. This only needs to
happen when something changes, such as a bond breaking, not every tick.

**Open.** How a piece's voxels are found cheaply. How a moving piece is stored is answered by D23, and
rotation by D22.

---

## D21 — Sand and gravel are interpretations, not definitions

*2026-11-05 · Phase 3*

Corrects D19, which described a sand voxel as standing for a bulk of grains.

FLUX has no sand or gravel material and no rule that defines them. Any material can be loose: copper
broken into single voxels or small clusters is copper sand. "Sand" is what a player calls the result
of a material and its bonds.

**Why.** Emergent, not scripted: looseness comes from `connections`, so every material gets it with
no extra rules.

**Limit.** The smallest possible piece is still one voxel.

---

## D22 — Rotation is planned, but not in the first version

*2026-11-05 · Phase 3*

The first version of moving pieces does not rotate. Rotation is still part of the architecture and
will be added later. A moving piece stores its voxels relative to one origin point, so the piece can
be moved, and later turned, without rewriting the position of every voxel.

**Why not now.** Falling, landing and picking up a piece do not need it, and it would make the first
version much harder to get right.

**Why it must be planned.** `EnergyEconomy.md` depends on water wheels and flywheels, which spin. Without
rotation, a piece hanging over a ledge also needs other rules to avoid staying there forever.

**Open.** How spinning machines work. How rotated voxels return to the grid is answered by D23.

---

## D23 — A moving piece is computed on paper; the grid holds only its footprint

*2026-11-05 · Phase 3*

**Status: working direction, not final.** To be tested by building, starting from single voxels.

While a rigid piece moves, its truth lives in a temporary record: position, velocity, angle, spin and
the data of its voxels. Each grid cell it covers holds only a marker naming the piece. When the piece
stops, its voxel data is written back to the grid and the record is deleted.

Voxels never rotate. They stay aligned cubes, and only their positions follow the piece, so a turning
piece appears as a stair-stepped shape. Each voxel's cell is calculated from the piece's original
shape, never from its previous cell, so rounding errors cannot build up.

**The rule that protects emergence.** Paper may only replace calculations whose answer it reproduces.
Anything it cannot represent, such as a bond breaking, melting or a piece splitting, goes back to
voxels, and voxel rules decide.

**Why.**
- Every voxel in a rigid piece has the same motion, so one set of numbers gives the same result as a
  million. This is compression, not approximation, and it follows from D20.
- Simulating each voxel would be less real. A push travels one cell per tick, 0.6 m/s, so a 1 m rock
  would take about 1.7 s to react at its far end. Matching stone at about 4,000 m/s would take about
  6,700 passes per tick.
- Two voxels rounding into one cell delete nothing, because their data is in the record.

**How it works.**
- A move checks the cells the piece would enter, nearest first. Air: the piece moves in. Solid: that
  is the contact point. Checking every entered cell also prevents tunnelling at any speed.
- Only cells the piece enters or leaves are written. Markers inside it stay the same.
- In a straight move, cells entered equal cells left, so air from the front moves to the cells behind.
- Contact away from the center of mass produces spin, so tipping needs no special rule.

**Accepted costs.**
- Cost grows with speed. A 1 m boulder needs about 20,000 writes per cell moved, and about 320,000 per
  tick at 16 cells per tick. That is very roughly 10 ms, estimated, not measured.
- Heat inside a piece pauses while it moves in the first version.
- A rigid piece cannot slowly bend.

**Open.** How the marker is stored, and what it adds to per-voxel state. Water and sand in a piece's
path. Air when rotation makes cells entered and left differ. Heat in long-lived moving pieces, which
could run the same rule in the record's own small grid. Whether single loose voxels use a record or
swap in the grid. How impacts crack a piece, which needs bonds.

---

## D24 — Single voxels fall by swapping, first in the tick

*2026-11-06 · Phase 3*

An active voxel swaps with the cell below it if it is strictly denser. The swap exchanges `materials`,
`energies` and `temperatures`. Falling runs first in the tick, before `updateTemperatures`.

**Density, not material names.** The rule never names stone or air, so any material follows it. Strictly
denser matters: with equal densities allowed, resting stone would swap with stone and never sleep.

**Why first.** `deltaArray` holds pending heat by cell from compute until apply. A voxel moved between
them would leave its heat behind on the wrong cell.

**Staying awake.** `nextActive` works as a vote: any system that needs a voxel next tick wakes it there.
A voxel with no votes sleeps, and `applyHeatFlow` does not need to know why.

**New cells join `active` in a second step.** The falling pass wakes the new cell in `nextActive`. After
the pass has finished for all chunks, every index in `nextActive` is also woken in `active`.
- Pushing into `active` during the pass is undefined behavior if it is the list being looped over, and
  could move a voxel more than once in a tick.
- Merging afterwards means the new cell gets its heat calculated that tick, so a falling voxel keeps
  exchanging heat. Falling and heat alternate, one tick each, which stands in for them happening at once.

**Limits.** One cell per tick is 0.6 m/s, with no acceleration. A voxel only falls straight down, so a
one-voxel tower can stand. Nothing wakes a voxel when the one under it moves away.

---

## Current voxel layout

| Field | Type | Bytes |
|---|---|---|
| `material_type` | `uint8_t` | 1 |
| `energy` | `uint32_t` | 4 |
| `connections` | `uint8_t` | 1 |
| **Total** | | **6** |

## Parked

- **Motion design questions.** To resume once a single voxel falls and lands:
  - Conflicts when two voxels or pieces want the same cell.
  - Energy accounting: height to speed to heat, and what the conservation check adds up. Proposed, not
    decided: height energy is a whole number per material and cell height, a piece stores its
    mechanical energy as an integer, and landing heat is integer subtraction.
  - How moving voxels stay in the active list, and where velocity data lives.
  - Whether D23 holds up, and its open items.
  - How a piece's voxels are found (D20) and the tipping rule.
  - Piles sliding: a blocked voxel tries the four diagonal cells below it. Needs fair direction choice
    and neighbours waking when a voxel moves away.
  - Spinning machines (D22).
- **Electrical energy.** Current is driven by potential difference, a new kind of quantity, not more
  energy. Belongs to the Electrical Age.
- **Slow ticks for slow regions.** Instead of freezing a low-gradient region, tick it less often with
  a larger `dt`. Stone stays stable up to ~18 s per step. Could cost less and flatten frozen hills
  (D17), since larger steps also lower the difference at which flow rounds to zero. Not designed or
  tested yet.
