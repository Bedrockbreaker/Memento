---
aliases:
  - Save Graph
tags:
  - TODO
  - programming
---
# Save Graph

## Overview

The [Memento System](MementoSystem.md) captures and restores world states as "mementos" (see link for terminology), forming a directed acyclic graph (DAG) of persistent state transitions. Unlike traditional save/load systems, which rely on opaque serialization snapshots, the Memento System models, how, why, and when a state is reached. This enables the game to respond not just to outcomes, but to decision lineage, temporal behavior, and repeated revision.

---

## System Architecture

### Graph Structure

- Modeled as a Directed Acyclic Graph (DAG).
	- Nodes represent distinct game states.
	- Edges represent player actions or system triggers that caused transitions.
- Each node contains:
	- Unique id
	- Serialized state payload (see Memento Format below)
	- Timestamp of capture
	- Triggering action metadata (e.g. "Used memory object", "Narrative auto-trigger", etc.)
	- Ancestral lineage: a pointer to the previous memento, and a second pointer to the restored memento
- Graph is stored as a mutable in-memory structure with disk-backed persistence.

### Runtime Behavior

On Capture:
1. Serialize current game state into a new Memento.
2. Link to active node via new edge.
3. Append node to the graph.
4. Emit events for downstream narrative systems (e.g. character memory evaluation, timeline reactivity).

On Restore:
1. Load target memento.
2. Traverse backward from current node to target; log transition as Loopback.
3. Reconstruct game state.
4. Register causal trace for use in consequence systems.

## Memento Format

Each memento consists of:

```json
{
	"id": "01234567-89ab-cdef-0123-456789abcdef", // Random UUID
	"timestamp": "12345678", // Milliseconds since game started across all saves
	"primary_parent": "11234567-89ab-cdef-0123-456789abcdef", // Previous save
	"secondary_parent": "21234567-89ab-cdef-0123-456789abcdef", // Restored save
	"delta": {
		"player_state": { /* health, inventory, location */ },
		"world_state": { /* entity flags, door states, puzzle status */ },
		"narrative_state": { /* dialogue trees, known facts, seen events */ }
		// ...
	},
	"metadata": {
		"context": "manual", // Player-initiated capture
		"tags": ["loopback"]
	}
}
```

## Serialization & Storage
