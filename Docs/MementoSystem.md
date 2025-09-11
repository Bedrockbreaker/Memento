---
aliases:
  - Memento System
tags:
  - design
---
# Memento System

## 1. Overview

The Memento System reworks traditional save/load mechanics as diegetic, narratively and systemically consequential actions. It treats saving as "capturing a memento" and loading as "restoring a memento," embedding these functions directly into the player's interaction space and the game's world logic.

The Memento System doesn't function as a save snapshot system, or even a linear series of snapshots. Instead, it's aware of revisits to previous saves, tracking not just what changed on the second visit, but how and why changes occurred, enabling narrative and systemic reactivity to both player choices and their temporal manipulations.

---

## 2. Design Goals

- **Embed Persistence** into the diegesis; saving/loading must feel like a thematic and mechanical action
- **Reward Reflection**, not just optimization; returning to prior choices must be laden with consequence, not convenience
- **Surface Temporal Authorship**; the player not only chooses what happens, when and why those choices are revisited
- **Enable Emergent Narrative** through consequences that reflect both the altered decision and the act of revision

## 3. Terminology

| Term          | Definition                                                                                       |
| ------------- | ------------------------------------------------------------------------------------------------ |
| **Memento**   | A snapshot of the world state, roughly equivalent to a normal save in other games.               |
| **Capturing** | Creating a memento from the current game state.                                                  |
| **Restoring** | Reverting the current state to a previously captured memento.                                    |
| **Node**      | A single memento in the save graph; contains state, transition metadata, and unique identifies.  |
| **Edge**      | A directional transition between mementos, annotated with the action that caused it.             |
| **Thread**    | A linear sequence of player actions from one memento to the next.                                |
| **Branch**    | A new timeline fork created when restoring and making different choices.                         |
| **Loopback**  | A return to a previous node; typically accompanied by world recognition or narrative adaptation. |
| **Fray**      | A detected pattern of indecision or exploitation (e.g. repeated restores to avoid consequence).  |

## 4. Player-facing Mechanic

- "Mementos" are physical representations of lived experience -- paintings hung in a surreal gallery space. Interacting with them initiates temporal reentry (restores a memento).
- UX Feedback includes changes to "world tone" (dialogue, post-processing, system responses), character behavior, and environmental detail. Consequences should always be understood to have occurred.
- Players cannot reenter endlessly without systemic consequence. Cost, instability, or altered perception guard against abuse.
- Certain NPCs (Atticus) or systems track and comment on the player's use and disuse of mementos.

## 5. Mechanic Structure

The mechanic is constructed through a "graph" of nodes and edges.

- **Nodes**: World states (level snapshot + narrative flags + player context)
- **Edges**: Save/load events
- **Metadata**: Timestamps, branch origin, reversal markers

The player's traversal forms a nonlinear path through the game's decision space, with the system capturing both structural (what changed) and behavioral (why and how) dimensions of play.

## 6. Game States & Memento Interaction

### 6.1. Capturing Mementos

Triggers:

- Key narrative triggers
- Player-initiated events (i.e. explicitly painting a new gallery painting)

Representation:

- A new painting appears in the gallery
- Title, timestamp, and room encoded visually

Constraints:

- Limited to major decision moments (prevents spamming)
- May cost resources or incur narrative strain if overused

Scopes of individual data captured:

- **Normal**: Traditional behavior of how time works. Actions have consequences in the current timeline.
- **Local**: Has zero effect on past or future nodes. "Ephemeral."
- **Global**: Overwrites past and future nodes. Can alter outcomes and events in different timelines and the past/future.

### 6.2. Restoring Mementos

Triggers:

- Initiated by player within gallery space
- Scripted actions

Representation:

- Restores world to saved state, but not cleanly -- partial memory, residual echoes, and inter-system inconsistencies remain.
- Small cutscene plays that transports player "through" the gallery painting

Narrative Response:

- Characters track the number of nature of memento reentries.
- Some lines or interactions only unlock if the player has reconsidered past choices.
- Repetition yields both insight and risk -- some characters become suspicious, others sympathetic.

Systemic Response:

- Environmental elements shift (e.g. wilted plants, cracked mirrors) to reflect instability.
- Puzzle logic may alter subtly based on the number of prior attempts or timeline branch.
- World state layers accumulate -- no "perfect reset."

The system evaluates on four axes of consequence:

- **Branching**: Traditional divergent outcomes from player choices.
- **Restoration**: Reactions to the act of returning, regardless of new choice made.
- **Post-Restoration Choices**: Reactions to choices that conflict (or agree) with previously made choices.
- **Pattern Detection**: Systemic response to behavioral trends (e.g. frequent reentry, indecision loops).

## 7. Player Experience Goals

1. **Reframe Save/Load as Diegetic Verb**
   
   The player should feel saving is part of the story, not outside of it.

2. **Encode Regret and Reflection**
   
   The player should feel emotional and strategic weight in revisiting choices.

3. **Encourage Narrative Experimentation, Not Min-Maxing**
   
   The player should treat saves as an expressive tool, not a retry button.
   
4. **Prevent Abuse Through Systemic Feedback**
   
   The player should be able to self-regulate their behavior due to meaningful, escalating consequences.

## 8. UX & Feedback

### Capture Feedback

- Visual: Color grading shifts subtly between timelines; gallery painting palette changes with instability.
- Audio: An audible whoosh.
- World: Environmental entropy or clarity communicates narrative stability.

### Restore Feedback

- Visual: Temporal distortion effect, environmental flicker or decay.
- Audio: Reversed chime or whoosh. Afterward during gameplay, layered or doubled voice lines signal memory dissonance.
- World: dialogue from self or others reacting to altered continuity, systems respond differently.

### HUD/UI

- No "save/load" buttons. Players access mementos only at paintings in the gallery.
- Timeline map shows visited branches abstractly (as a branching timeline of paintings across a wall).

## 9. Edge Cases

| Edge Case                            | Design Mitigation                                                             |
| ------------------------------------ | ----------------------------------------------------------------------------- |
| Infinite reload looping              | Diminishing returns; increased instability; narrative hostility.              |
| Save before every decision (min-max) | Save cost; limited save slots; recognition + reaction from the world.         |
| Abuse to bypass puzzle systems       | Puzzle state partial retention; soft reset with history-aware solution paths. |
| Emotional detachment from undoing    | Design ensures no decision can be fully undone without residual traces.       |

## 10. Integration Points

- **Narrative System**: Dialogue and story events reference DAG traversal history.
- **Puzzle System**: Puzzle states may preserve elements of past attempts or reveal alternate solutions based on timeline.
- **AI/Character Logic**: Atticus and other NPCs hold memory across branches and respond to player behavior patterning.

## 11. Future Expansions

- **Memento Corruption**: Overuse of temporal reentry creates ghost nodes or distorted states (completely detached timelines, perhaps with pre-filled, scripted save data).
- **Reactive Gallery**: Gallery itself begins to alter, with paintings reacting to timeline density.
- **Multiplayer**: Timeline consensus and reckoning.
- **Probabilistic Mementos**: Randomly imperfect captures/restorations.
