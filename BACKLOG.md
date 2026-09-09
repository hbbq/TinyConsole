# Backlog

## Ideas

- **SRB procedural levels and difficulty progression:** Keep level 1 fixed and generate subsequent levels procedurally. Increase obstacle frequency both within each level and across successive levels. Start with one special-level theme: no bottom ground, recurring at a fixed interval still to be decided. Decide the difficulty curve and rules for playable obstacle combinations within the ATtiny85's flash and SRAM limits.
- **Redesign SRB level 1:** Rework the fixed, hand-authored first level while keeping it non-procedural. Explore its layout and how it introduces the challenges found in later procedural levels.

## Todo

## In Progress

## Done

- **SRB flying enemies:** Implemented metadata-selected flyers (`10`), full-byte `0b11111111` level end, vertical movement every 16 ticks, shared enemy blocking, and lives-display protection. Type/direction share slot 8 and reset on entry/reuse; level layouts unchanged. ATtiny85 build: 7,670 B flash (+174 B), 57 B SRAM (unchanged). Passed all 104 existing patrol AVR simulation checks; user manually tested flying enemies and reported good results (see `test/test_srbpatrol/README.md` for the regression checklist).

- **SRB ground enemy patrol:** Implemented left-first patrol on the existing 16-tick cadence, wall/enemy turns, terrain-only ledge landing checks, and both-edge deactivation. Directions use slot 8 and reset on level entry/slot reuse; gravity, spawning, scrolling, damage, and stomps are preserved. Passed 104 AVR instruction-simulation checks, including launcher, rendering, buttons, and restart. ATtiny85 build: 7,496 B flash, 57 B SRAM (flash +158 B, SRAM unchanged). Simulator uses an ATmega2560 instruction target; Wokwi/hardware playthrough remains unrun. See `test/test_srbpatrol/README.md`.
