---
aliases:
  - Memento System
tags:
  - TODO
  - design
---
# Memento System

## Overview

The Memento System reworks traditional save/load mechanics as diegetic, narratively and systemically consequential actions. It treats saving as "capturing a memento" and loading as "restoring a memento," embedding these functions direclty into the player's interaction space and the game's world logic.

The Memento System doesn't function as a save snapshot system, or even a linear series of snapshots. Instead, it's aware revists to previous saves, tracking not just what changed on the second visit, but how and why changes occurred, enabling narrative and systemic reactivity to both player choices and their temporal manipulations.

---

## Design Goals

- Reframe saving/loading as a part of the narrative and systemic affordances.
- Enable player interaction with time and memory as in-world concepts.
- Track choice lineage and revisitation behavior.
- Influence narrative and gameplay based on player use patterns.
- Make capturing/restoring mementos consequential and irreversible in certain contexsts.

## Terminology

**Memento**: A snapshot of the world state, roughly equivalent to a normal save in other games.
**Capturing**: Creating a memento from the current game state.
**Restoring**: Reverting the current state to a previously captured memento.
**Node**: A single memento in the save graph; contains state, transition metadata, and unique identifies.
**Edge**: A directional transition between mementos, annotated with the action that caused it.
**Thread**: A linear sequence of player actions from one memento to the next.
**Branch**: A new timeline fork created when restoring and making different choices.
**Loopback**: A return to a previous node; typically accompanied by world recognition or narrative adaptation.
**Fray**: A detected pattern of indecision or exploitation (e.g. repeated restores to avoid consequence).

## Player-facing Mechanic

1. Players capture mementos via specific in-world actions (i.e. interacting with a specific object and reaching narrative checkpoints).
2. Restoring a memento requires an equivalent in-world action.
3. Each action is acknowledged diegetically by the world and characters.
4. Certain NPCs or systems track and comment on the player's use of mementos.

## Consequence Modeling

Restoration is not a neutral action. Each use is tracked, interpreted (patterns like frequent reversion, hesitation, or timeline pruning affect world behavior), and constrained (limited number of restorations in certain situations, cost-gated restores, or permanent consequences).

Characters may remember prior timelines, recognize paradoxes, or suffer memory damage.

## UX & Feedback

### Capture Feedback

- Visual: screen vignette, memory motif overlay.
- Audio: chime or voice whispering.
- World: object changes appearance to reflect memory binding.

### Restore Feedback

- Visual: Temporal distortion effect, environmental flicker or decay.
- Audio: reversed chime or voice whispering.
- World: dialogue from self or others reacting to altered continuity, systems respond differently.

### HUD/UI

- No "save/load" buttons. Players access mementos only at in-world anchors.
- Timeline map shows visited branches abstractly (as constellations).

### Narrative Integration Hooks

- Characters with temporal awareness comment on restored states.
- World artifacts retain knowledge of prior threads.
- Events change based on depth of memory interference.
- System enables explicit storytelling around regret, denial, determinism, and multiplicity.

### Edge Cases & Resilience

- Infinite reload loops detected via restory frequency and pattern.
- System injects feedback when abuse is suspected (e.g. timeline bleed effects).
- Partial corruption effects simulate narrative wear on memory.

### Extensibility

- Designed to support episodic content or large-scale branching expansions.
- State serialization decoupoled form object logic (via versioned interfaces).
- Memento nodes can story developer annotations for debugging or future narrative hooks.

### Future considerations

- Partial memento restoration (e.g. revert only one system's state).
- Multiplayer timeline concensus.
- Probabilistic state restoration (quantum memory decay or randomly imperfect recall).
