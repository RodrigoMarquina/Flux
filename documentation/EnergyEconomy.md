# The Energy Economy

This document defines the resource model of FLUX. It is not a feature list. It is the set of
physical rules the world runs on, from which the gameplay, the progression, and the performance
budget are all expected to emerge.

The design goal that produced it: in most simulators, computational cost is the developer's
problem, hidden from the player, and exceeding it is a failure the fiction does not account for.
FLUX inverts that. The limits of the world are made visible, physical, and learnable, so that
managing them becomes a skill the player develops rather than a wall they hit.

---

## 1. The two laws

Everything below follows from two statements.

**First law — energy is conserved.** It cannot be created or destroyed, only moved and converted.
Nothing the player builds can ever change the total.

**Second law — exergy is destroyed.** Every irreversible process degrades energy from
concentrated and useful to diffuse and useless. This is one-way. It never reverses on its own.

| | Behaviour |
|---|---|
| Energy | Conserved. Constant. Cannot decrease. |
| Exergy | Destroyed by every irreversible process, including doing nothing. |

Because energy can never decrease, it cannot be scarce, and a quantity that cannot be scarce
cannot be a resource. **The resource in FLUX is exergy.**

---

## 2. What exergy is

Exergy is the maximum useful work extractable from a system as it comes to equilibrium with its
surroundings. Informally: not how much energy there is, but how much of it can still do
something.

The water analogy:

- Energy is how much water you have.
- Exergy is how high up it is.
- Ambient is sea level.

A lake at sea level holds enormous water and can turn no wheel, because there is nowhere lower
for it to fall. A bucket on a mountain holds almost nothing and can do real work. Only the drop
matters. For heat, "height above sea level" is temperature above ambient.

A hundred ingots at 100 °C and one ingot at 10000 °C can hold the same joules. Only the second
can melt copper. Concentration is quality.

Roughly, a joule of heat at 2000 K is worth about 85% of itself as work; at 400 K, about 25%; at
ambient, zero. Same joule, different worth.

**Exergy is also a statement about time.** It measures how much the future can still differ from
the present. A world at equilibrium has no possible future that differs from now — nothing can
happen. Zero exergy means zero gameplay, which is the same observation as heat death.

### Exergy is never stored

Exergy is not a field on the voxel and not a global counter. It is a property of how energy is
*arranged*, derivable from temperature, material and state whenever it is wanted.

The precedent is potential energy in rigid body dynamics: nobody stores `potential_energy` on a
body. Mass and position are stored, and `mgh` is derived. Potential energy is completely real and
completely causal, and it is not a variable.

This matters architecturally. Per the project rule — *nothing is stored that can be derived* —
exergy stays out of the voxel struct. It is a lens for designing rules and a number for the
player-facing readout, nothing more.

**The heat transfer rule is already exergy destruction.** "Heat flows from hot to cold at a rate
proportional to the difference" *is* the second law implemented. Every time two voxels equalise,
exergy was destroyed. No system needs to be added for it.

If a `world_total_exergy` variable ever appears, it is a scripted global standing outside the
physics, and it is wrong.

---

## 3. Stock and rate

A **stock** is an amount held: joules, a forest, a full reservoir. Spent once.

A **rate** is an amount per unit time: watts, litres per second, trees grown per day. Perpetual,
but capped.

Savings versus salary. Battery versus wire.

Machines do not consume joules, they consume watts. A furnace needing a certain heat input cannot
run on a large stock delivered slowly. The player can be sitting on an enormous reserve and still
be unable to run the workshop, because they cannot get it out fast enough. That is an engineering
problem with solutions — concentrate it, buffer it, insulate it — rather than a wall.

"You cannot have a million machines running" is a **power** limit, not an energy limit.

---

## 4. The world boundary

The world is **finite and bounded** (a sphere) but **not closed**. It has exactly two boundary
terms, and they are the only places where energy enters or leaves.

**Solar influx.** A continuous rate, not a stock. Low-grade and diffuse — it arrives spread thin
over a large area at low temperature, and concentrating it into something that can smelt metal is
itself the engineering problem. It cannot be trivially exploited and it cannot be increased.

**Radiative outflow.** The world radiates heat to space. This is not a leak to be prevented — it
is the **cold sink**. Carnot efficiency is `1 - T_cold / T_hot`; with no cold reservoir to reject
waste heat into, no engine works at all, regardless of how much energy is available. Night is what
recharges the cold.

### Why the world was not made closed

A sealed world was the original intent, on the reasoning that openness means leakage and leakage
means an unwinnable state. That reasoning is inverted. Irreversibility is what traps you, and
irreversibility lives inside the box. A closed system is the only configuration in which heat
death is guaranteed, and it makes early mistakes permanent — which inverts the difficulty curve,
making the game harshest when the player knows least.

Three further arguments settled it:

1. **Wood must be recoverable.** When wood burns, the energy is still in the world, spread thin
   and worthless. Returning it to wood means *re-concentrating* it, which is uphill, and uphill
   must be paid for with something higher-grade. In our universe that payment is sunlight. There
   is no other mechanism that is not an invented rule, and inventing one violates *rules, not
   interactions*.
2. **The pre-industrial tier depends on it.** Rivers, rain and wind are solar-driven. Without a
   sun, rivers run dry permanently and never refill, and water wheels and windmills — the tier
   between muscle power and steam — are lost.
3. **The cold sink is required.** See above.

### The sun does not inflate the world's energy

At steady state, energy in equals energy out, and total world energy is roughly constant.

A day in the world, with the sun delivering some quantity Q:

| | Energy | Exergy |
|---|---|---|
| In — sunlight, high quality | +Q | large |
| Out — radiation to space, at ambient | −Q | ≈ 0 |
| Net | **0** | large influx, nearly all destroyed by day's end |

Energy in equals energy out, but exergy in is enormous and exergy out is essentially nothing,
because outgoing radiation leaves at ambient temperature and energy at ambient can do no work.

**The sun does not add energy. It replaces quality.** The world is a machine that takes in
high-grade energy and emits the same quantity of worthless energy. The difference is destroyed
inside, every day, whether or not anyone uses it.

### Ambient temperature is emergent

A hotter body radiates far more strongly (as `T⁴`). If energy begins to accumulate, the world
warms, warming makes it radiate harder, and it settles at an equilibrium temperature.

Ambient is therefore an emergent property of solar output and radiating surface, **not a constant
typed into the code**. A consequence worth having: heavy industrialisation raises ambient, which
raises `T_cold`, which makes every engine in the world permanently less efficient.

---

## 5. Stocks are delays, not additions

Energy flows through the world on its way to space. Some of it takes a detour — years through a
tree, millennia through a coal seam — before continuing. Nothing accumulates permanently.

Storage happens only while a stock is *filling*. Bare rock growing into forest genuinely banks
solar energy; a mature forest is at steady state, growth equalling decay, net storage zero. For
scale: Earth's entire photosynthesis captures on the order of 0.1% of incoming sunlight. Every
forest and every crop, one part in a thousand.

Quality can be stored. Quantity cannot, because it leaves.

### The three sources

| Type | Source | Character |
|---|---|---|
| Stock | Planetary core | Huge, deep, concentrated, high-grade, finite. The endgame prize. |
| Stock | Wood, coal, buried fuel | Concentrated, accessible, finite. The early bootstrap. |
| Rate | Sun | Perpetual, low-grade, diffuse, rate-capped. Never runs out, never scales. |

Early game lives on stocks because they are concentrated and easy. As stocks deplete, the player
must either chase the largest remaining one downward into the core, or learn to live on the flow —
capturing sun, wind and water well enough to run a civilisation on what arrives each day rather
than on what was buried.

That is the real energy transition, arrived at through physics rather than written into a quest.

---

## 6. Sustainable yield versus drawdown

Every renewable stock has a regeneration rate.

- Extraction ≤ regeneration → sustainable indefinitely. The player is living on the flow.
- Extraction > regeneration → the stock is being eaten, and a countdown is running whether or not
  the player can see it.

Nobody designs this rule. It falls out of a finite stock with a finite refill rate.

It also delivers the required failure mode: overharvesting is **not** a soft-lock, it is a setback
measured in time. Burn every forest and the player does not lose — they wait. A difficulty spike,
not a wall, and it teaches the lesson exactly once.

This is also the Bronze-to-Iron pressure with no quest anywhere: wood regrows too slowly to
sustain a real furnace economy, so the player is pushed toward denser, deeper, non-renewable
stocks. The simulation makes the next step necessary.

---

## 7. Progression is efficiency, not quantity

The world's exergy is always falling. That cannot be stopped. What rises is the player's
capability per unit of exergy, and it must rise faster. **The game is that race.**

The same unchanging goal — smelt one copper ingot — across the eras:

| Era | Method | Fraction of the fuel's exergy reaching the ore |
|---|---|---|
| Fire | Open campfire | ~1–2%, nearly all rising as hot air and light |
| Clay | Pit kiln with a lid | ~5%, loss upward contained |
| Bronze | Furnace with bellows | ~15%, forced air raises temperature, which raises quality, which raises the Carnot ceiling |
| Iron | Insulated furnace, preheated air | ~30%+, exhaust warms incoming air before escaping |
| Steam+ | Cascading — smelter exhaust drives an engine, engine exhaust dries clay, that exhaust warms a home | each joule does work repeatedly on the way down |

Same ingot, roughly fifty times less exergy at the end than at the start.

Two consequences:

**The tech tree is a list of conversions, not recipes.** Chemical→thermal (fire). Thermal
concentration (kiln). Gravitational→mechanical (water wheel). Thermal→mechanical (steam — the
large one, requiring the player to genuinely understand that a cold side is necessary).
Mechanical→electrical (generator). Each unlock is a new arrow between energy forms, and each new
arrow opens reservoirs that were previously untouchable.

**The failure mode is waste, not loss.** A bad player does not lose the game. They spend 10% of
the world doing a job a good engineer does for 0.5%. Attributable, learnable, recoverable, and
visible — which was the original requirement.

### Machines have a capital cost in exergy

A charged flywheel, a hot furnace, a pressurised boiler — each is a quantity of high-grade energy
locked in inventory and unavailable everywhere else. Infrastructure competes with operations for
the same pool. This is a deliberate strategic axis, not an accident.

### Every machine is a tollbooth

A solar panel takes nothing extra. That sunlight was striking the ground and becoming waste heat
regardless. The panel intercepts energy already on its way through and makes it do a job before it
continues to its inevitable fate. Same energy, same destination, same total — work done on the way
down.

This is the pattern for everything the player will ever build. **You never add to the flow. You
put a wheel in it.** The tech tree becomes: where along the cascade can I stand, and how good is
my wheel.

The water wheel is the clearest case. Sun evaporates water, water rains on high ground, gravity
brings it down through the wheel, the player takes a cut of the fall. Nobody scripts a
hydrological cycle; it appears on its own from conservation of mass plus heat plus gravity. The
water wheel is a solar engine in disguise.

---

## 8. Perpetual motion must be impossible by construction

Every transfer must lose something in the moving. Friction turns motion into heat, conduction
bleeds heat to neighbours, electrical resistance dissipates. Efficiency may approach 100%
asymptotically, with the cost of each remaining percent exploding, and must never reach it.

**These loss terms are load-bearing, not flavour.** If heat conduction is implemented as pure
exchange with no dissipation, or mechanical motion with no friction, a closed loop with net
positive output exists and players will find it. The moment they do, the economy is over.

Perpetual motion must be impossible *emergently* — a consequence of every rule leaking a little —
and never blocked by a special-case check.

### The cautionary case

Minecraft's infinite water source: two source blocks make a third, and water is created from
nothing. A first-law violation baked into the base game as a convenience. Everything downstream
inherits it — in Enigmatica 2 Expert, infinite water becomes infinite essence becomes infinite
crops becomes infinite energy. The mod authors did not design a broken economy; they built
carefully on a foundation that was already broken, and every loop became a money printer.

Note what it does to the game. The content is the logistics puzzle on the way to infinite energy.
The moment it arrives, the game is structurally over — every remaining problem collapses to "build
more of these." Scarcity was the content.

In FLUX this cannot happen by construction. Water is voxels with conserved mass: it can be moved,
evaporated and condensed, never created. A water wheel eventually runs its river dry unless
something lifts that water back up the mountain — and what lifts it is the sun.

---

## 9. The conservation invariant

Every internal rule conserves energy exactly. Conduction moves it, combustion converts its form,
friction turns motion into warmth — none of them change the total by even a rounding error.

The only two exceptions are the boundary terms: solar influx and radiative outflow.

This gives a hard, testable assertion:

> Sum the world's energy. Subtract solar influx. Add back radiative loss. If the number moved,
> **there is a bug.**

Not a design question — a bug. Most physics simulations drift silently because nobody checks
this. This check should exist *before* the heat transfer it validates, and it will catch an entire
class of errors — bad float handling, double-counted transfers, asymmetric neighbour updates —
that would otherwise take weeks to find.

---

## 10. Computational cost is the same problem

The original goal was to hand performance responsibility to the player. It is achieved by the
exergy economy without a separate system.

What the engine pays for is voxels in *disequilibrium* — gradients actively flattening, neighbours
waking neighbours. What an exergy leak physically *is* is a gradient flattening without useful work
extracted. They are the same phenomenon.

The campfire player's world is expensive to simulate: huge volumes of air heating, gradients
everywhere, thousands of voxels awake. The insulated furnace keeps its heat in a small contained
volume. **The efficient player's world is cheap to compute for the same reason it is cheap in
exergy.** The physics bills the player automatically; no fake computational budget is bolted onto
the fiction.

Be honest about the limit: this is a strong correlation, not an identity. A large, perfectly
efficient factory still costs more compute than a small wasteful hut. What the engine charges for
is *activity*; what the player learns to minimise is *waste per unit of useful work*. Where they
diverge, the bounded active list handles it — see below.

### The active list is a game design decision

If the computational budget is a mechanic, the active list is not "everything awake this tick."
It is a **bounded, prioritised queue with an eviction policy**. When far more voxels want
simulation than can be afforded, something decides who wins — and that policy lives in the hottest
loop in the engine.

Open questions, to be settled in Phase 3: what the ranking is (proximity, energy, age of
activation), whether the player can influence it, and whether the budget is global or per-region.
Global means overloading a forge degrades a river on the far side of the world — coupling between
unrelated things, and a griefing vector the moment there is a second player. Per-region is
spatially legible and maps onto the chunks that already exist.

### Legibility

For the player to solve an overload, they must perceive the cost, attribute it to a cause, and
predict it before committing. If the only feedback channel is a dropping frame rate, the game has
punished them by degrading the exact interface they need in order to diagnose the problem — the
worse they do, the less able they are to fix it. That is a trap, not a skill curve.

Computational load needs an in-world representation the player *reads*, not one they *suffer*.

---

## 11. Level of detail, in space and in time

Detail is a resource, spent where the player is and saved everywhere else. This is not an
optimisation bolted on; it is *computation is the boundary of reality* in practice.

**Spatial LOD** — distant chunks simulated and rendered at coarser voxel resolution.

**Frozen chunks** — a chunk with nothing active in it stops being simulated entirely, at zero
cost. This is what makes a large world possible.

**Temporal LOD** — a genuinely frozen chunk means time has stopped there: leave a fire burning,
return an hour later, find it untouched. Instead, on waking, apply *one* cheap pass approximating
what the elapsed time would have done — relax temperatures toward the local average, burn down the
fuel, settle loose material — rather than replaying thousands of ticks. Elapsed absence has
consequences, paid for once per chunk instead of once per tick.

Coarser space where the player is not looking; coarser time where the player is not present. One
principle, two axes.

Note that sleeping chunks are a suspension of thermodynamics for computational reasons. That is an
accepted, deliberate approximation — the simulation is not a replica of the real world, and this
is one of the places where that licence is spent.

---

## 12. Open questions

- **Active list policy.** Ranking, eviction, player influence, global versus per-region budget.
- **Legibility.** What the in-world representation of computational load actually is.
- **Sky exposure.** Both boundary terms need to know whether a voxel is exposed to the sky. That is
  a cousin of the occlusion culling problem already open in the README — not the same query
  (camera visibility versus visibility from above), but the same underlying primitive. Worth
  knowing before committing to a chunk representation.
- **Soil and biomass.** Whether tree growth draws on a finite soil-fertility stock seeded at
  worldgen, which would make soil a resource and give agriculture a physical basis.

---

## Summary

| Decision | Resolution |
|---|---|
| Resource unit | Power — a rate, not a stock |
| Real currency | Exergy: energy × quality, quality being distance from equilibrium |
| Where exergy lives | Nowhere. Derived from temperature, material and state; never stored |
| What worldgen distributes | Disequilibrium — hot core, cold surface, fuel, water at height — not a number |
| World boundary | Bounded but open: sun in, waste heat radiated out. Exactly two boundary terms |
| Energy sources | Sun (rate, low-grade) · core (stock, high-grade) · biomass (slow rate) |
| Ambient temperature | Emergent from solar input and radiating surface, not a constant |
| Progression axis | Efficiency per unit exergy. The tech tree is a list of conversions |
| Failure mode | Waste — 10% of the world for a 0.5% job. Never a soft-lock; setbacks measured in time |
| Perpetual motion | Impossible emergently, via loss terms in every transfer rule. Never special-cased |
| Absent regions | Frozen, with a coarse catch-up pass on wake — temporal LOD |
| Compute link | Free: exergy leaks and simulation cost are the same phenomenon |
| Hard invariant | Internal rules conserve energy exactly; any drift is a bug |
