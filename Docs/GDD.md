---
aliases:
  - GDD
tags:
  - design
---
# Memento - GDD

> Narrative-Conscious Persistence Through Reframing Save/Load as a Mechanically Embedded, Systemically Causal, and Progression-Critical Player Action in Decision-Responsive Game Worlds

![Icon](Icon.png)

**Platform**: PC (Windows/Linux)
**Engine**: Unreal Engine 5.6
**Language**: C++ (select Blueprint for UI prototyping)
**Development Tools**: VSCode, GitHub, Twine
**Genre**: Experimental Narrative / Interactive Fiction / Puzzle
**Perspective**: First-person

---

## 1. High-Level Concept

Memento is a vertical slice of an experimental narrative game where save/load is not a menu function, but a core mechanic embedded in the story and world (called "mementos"). The navigates an emotionally reactive environment that remembers not just what the player did, but when they chose to revisit decision -- and why. The result is a systemic narrative experience that explores memory, regret, and agency through gameplay that bends time and causality.

## 2. Core Motivations

Where most games treat save/load as invisible scaffolding, Memento turns it into the centerpiece: a diegetic progression-critical mechanic that players use, abuse, and reflect upon -- with the game world watching, reacting, and evolving. The objective is not only to preserve continuity across nonlinear timelines, but to narratively and mechanically frame persistence itself as a meaningful act.

## 3. Core Design Pillars

1. Temporal Reentry as Agency

Players can re-enter past moments (saves), but the world responds not only to changed outcomes, but to the *act* of changing them.

2. Graph-Based World State

The game models progression as a directed acyclic graph (DAG), preserving the history and causal lineage of decisions.

3. Narrative-System Fusion

All systems -- dialogue, AI, puzzle logic -- reference the timeline graph to produce emergent and context-aware reactions.

4. Diegetic Save/Load

Reframed through in-world metaphors (i.e. gallery paintings), save/load is treated as a magical power.

## 4. Gameplay Overview

### 4.1. Core Loop

1. **Encounter** a scenario involving choice (dialogue, object, spatial puzzle)
2. **Act** on a choice, branching the world state
3. **Revisit** a prior moment via diegetic memory
4. **Observe** how the world and characters shift in response -- not just to the new decision, but to your return.

### 4.2. Controls (PC)

Standard 3D First-Person camera and movement system.

| **Input**    | **Action**      |
| ------------ | --------------- |
| WASD / Mouse | Navigate / Look |
| E            | Interact        |
| Q            | Cancel          |
| Esc (hold)   | Quit            |

## 5. Narrative Design

### 5.1. Theme

The narrative explores revisionism, memory recursion, and the ethics of regret. Rather than treating memory as immutable, the player's ability to reconsider becomes a source of both empowerment and dissonance.

### 5.2. Setting

A dusty, Victorian-style library, functioning as both a literal space and a metaphoric mindscape. Relics, echoes, and ghost-dialogue fragments reveal the memory architecture of the world and the player character. Players interact with the memento system (saving/loading) via gallery paintings.

### 5.3. Characters

- The Stranger (Silent Protagonist): An unknown passer-by who entered the library out of curiosity, but found no way back.
- Atticus: An NPC who exists within the library, reacting different depending on how (and how many times) you've interacted with them.

## 6. Systems Design

### 6.1. Persistence Graph

Implemented as a custom DAG.

- Nodes: World states (level snapshot + narrative flags + player context)
- Edges: Save/load events
- Metadata: Timestamps, branch origin, reversal markers

### 6.2. Save/Load as Mechanic

- "Capturing a memento" (saving): Performed at scripted narrative moments, as well as just before "traveling" to a previous memento. This binds the current game state to a new painting in the gallery to go to.
- "Restoring a memento" (loading): Diegetic "time walk" to a prior node. Reentry is costed, causing instability, altered NPC behavior, etc. Some partial states from prior nodes are always retained.

### 6.3. Choice & Consequence Model

Each player choice includes:

- **Type**: Branching (traditional binary or weighted choices), Timeline-Intrinsic (e.g. "have they reloaded?"), or Pattern-Driven (e.g. repeated behavior)
- **Scope**: Normal (only affects future nodes), Local (has zero effect on past or future nodes), or Global (overwrites past and future nodes)
- **Persistence**: Logged in a "choice ledger" (embedded within the DAG), referenced by later systems.

## 7. Moment-to-Moment Mechanics


| **Mechanic**      | **Description**                                                                      |
| ----------------- | ------------------------------------------------------------------------------------ |
| Gallery Paintings | Objects that anchor world states and allow temporal reentry                          |
| Dialogue          | Adaptive conversations that reflect not just choice, but sequence and context        |
| Puzzle            | A standard 3D spatial puzzle with multiple solutions that lead to different outcomes |

## 8. UX & Feedback Design

### 8.1. UI Philosophy

UI is to be minimal and abstracted -- it must feel like an extension of the world, not a developer overlay.

### 8.2. Feedback Cues

- Color grading shifts subtly between timeline states. Less saturated = more negative emotions intended by narrative
- Characters stutter or double-voice when dissonance is high
- Narrative callouts (e.g. "you chose differently this time...") reinforce awareness.

## 9. Technical Implementation

| **System**           | **Implementation**                                                                   |
| -------------------- | ------------------------------------------------------------------------------------ |
| Save Graph           | Custom C++ DAG extending Unreal's existing save system, with runtime edge evaluation |
| Narrative Logic      | C++ custom state interpreter using Twine-generated syntax                            |
| Timeline Debug Tools | In-editor viewer showing graph state and memory weights                              |
| Serialization        | Delta-based compression between nodes to minimize save overhead                      |
| Reentry Mechanism    | Blueprint/C++ hybrid for UI interaction and backend "commit"                         |

## 10. Content Plan (Vertical Slice)

1. **Intro Hall**: Establish tone and tutorial for movement/interactions
2. **Gallery**: First introduction to mementos; teaches the core mechanic; Intro to Atticus
3. **Indoor Greenhouse**: Choice-moderate sequence with multiple reentry points; contains main puzzle
4. **Atticus's Study**: Timeline reactivity and world acknowledgement; choice-heavy sequence 
5. **Hall Collapse**: Culmination where earlier choices and reentries unlock new escape paths.

## 11. Development Timeline

1. Milestone 1: Documentation, Basic Save/Load API, Initial Game Systems Implementations
	1. Sprint 1: Documentation
	2. Sprint 2: Save/Load Graph Backend, Editor Tools
	3. Sprint 3: Puzzle & Dialogue Systems
2. Milestone 2: Save Graph Reactivity
	1. Sprint 4: Dialogue Branching, State Reactivity
	2. Sprint 5: Puzzle Reactivity
	3. Sprint 6: Anti-Abuse Mechanics, Softlocks, Debugging
3. Milestone 3: Feedback & Refinement
	1. Sprint 7: VFX/Audio/UI Feedback, Replace Non-Diegetic Affordances
	2. Sprint 8: Refine Dialogue, Art Assets, Puzzle Polish, Playtesting
4. Milestone 4: Final Passes & Release
	1. Sprint 9: Stress Tests, Edge Cases, Finalize Developer Tools
	2. Sprint 10: Builds, Presentation Prep, Documentation Updates

## 12. Completion Criteria

1. Core Gameplay Mechanics
	- [ ] Does the game include at least one fully implemented instance of the save/load-driven mechanic?
	- [ ] Does the save/load system produce consequences that later future gameplay or narrative states?
	- [ ] Is the player clearly informed (diegetically or via feedback) that their use of the system has persistent consequences?
2. Narrative & Systems Integration
	- [ ] Are narrative events affected by the player's use (and non-use) of the core mechanic?
	- [ ] Is there at least one character or world system that acknowledges or reacts to timeline reentry to altered decisions?
	- [ ] Are player choices tracked across at least two playthroughs' states or reversions?
	- [ ] Does the game include at least one moment where mechanical behavior reinforces a narrative theme (e.g. regret, revisionism, determinism)?
3. Programming & Systems Architecture
	- [ ] Is the save/load system implemented as an integrated gameplay mechanic, not just an engine feature?
	- [ ] Is there a custom state tracking system (e.g. node graph, command history, or deltas) implemented to manage narrative and gameplay branches?
	- [ ] Does the game support serialization and restoration of world state across multiple progression points?
	- [ ] Is state persistence designed to be data-driven and extensible to future content without major rewrites?
	- [ ] Are game objects and systems properly decoupled to allow safe save/load at runtime?
4. Systems-Level Behavior
	- [ ] Are the consequences of saving/loading observable within the vertical slice (not implied)?
	- [ ] Does reloading affect not just values (health, location), but systemic world behavior (dialogue, state machines, etc.)?
	- [ ] Are at least two non-narrative systems (e.g. AI, combat, puzzles) impacted by save/load behavior?
	- [ ] Is the mechanic resilient to abuse (e.g. no infinite reloading to bypass systems without narrative costs)?
5. UX, Feedback, & Presentation
	- [ ] Is the player's interaction with the save/load system clearly communicated through UI or in-world feedback?
	- [ ] Does the game avoid using standard save/load language and visuals, reinforcing the mechanic as an in-universe feature?
	- [ ] Is there visual or auditory feedback that reinforces the concept of timeline alteration or decision revision?
	- [ ] Are in-game prompts or responses tailored based on the player's repeated or altered actions?
6. Tools, Testing, & Debugging
	- [ ] Does the tool side include developer-facing utilities for testing save/load state or debugging branching logic?
	- [ ] Is there a method to inspect or log which narrative or gameplay states have changed due to timeline alteration?
	- [ ] Can the player experience at least one divergent outcome based solely on the use of the core mechanic?
	- [ ] Has the system been tested for edge cases, including repeated reloading, skipping intended states, or premature exits?
7. Project Scope & Readiness
	- [ ] Does the vertical slice demonstrate a self-contained sequence that showcases the mechanic's full loop?
	- [ ] Is the game stable enough to be played from beginning to end without critical blocking bugs?
	- [ ] Is the pitch-able version of the game playable without external explanation (i.e. it teaches its own mechanics)?
	- [ ] Has all placeholder content (text, UI, art) been replaced with prototype-level or better polish?
