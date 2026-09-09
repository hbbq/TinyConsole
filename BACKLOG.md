# Backlog

## Ideas

- **SRB hand-authored milestone levels:** Consider keeping the current Super Mario Bros.-inspired level 1 in the personal version and adding fixed, hand-authored levels at selected points among procedural levels. Use these for deliberate pacing, memorable challenges, or introducing mechanics. Revisit their placement, themes, and whether to use original designs or tributes; the current level's suitability for distribution remains unresolved.

## Experiments

## Todo

- **SRB procedural enemy mix and spawn eligibility — DEV:** For procedural levels, choose flyers with probabilities 1/8, 2/8, 3/8, and 4/8 for the four five-level difficulty tiers (cap at tier four from level 16); add 1/8 on every fifth, bottomless level. Flyers are therefore possible from level 2. Use seed/column-deterministic random selection so retries and repeated column lookups agree. With zero-based rows, a flyer spawns at y=3 only if y=3 is clear and at least one of y=2 and y=4 is clear. Patrols require at least one terrain block in their column. Skip an ineligible selected type rather than substituting another enemy. No generated enemies in columns 0..15 or the fixed ending (238..255); preserve fixed level 1. Retain the existing four-enemy limit and movement behavior. Verify the spawn-rule combinations, tier/cap and bottomless probabilities, and deterministic placement; build `pio run -e attiny85` and report flash/RAM. Use the experiment's initial overall threshold (2 + tier + (x >> 6)) out of 64 per eligible column, and keep columns 232..237 enemy-free as part of the exit apron. Validate enemy-inclusive routes without forced damage or stomps; terrain-only reachability does not establish this. See experiments/srb-generators/README.md.

## In Progress

## Done

- **SRB increase procedural terrain intensity and motion readability:** Raised early terrain activation from 2/8 to 5/8 and hardened later tiers with forced wide gaps, tall steps, and hole-plus-step patterns. Added recurring row-4 landmarks, limiting featureless ground to 14 columns while preserving the separate enemy curve. Passed 3,584 routes, 1,856 transitions, 33,964 AVR witness checks, and 108 gameplay/launcher checks. ATtiny85 build: 7,906 B flash, 57 B SRAM.

- **SRB safe procedural terrain and 16-level progression:** Replaced the old level-2 helpers with the proven independent, pattern, and bottomless generators; fixed level 1 remains, levels advance through 16 and wrap, and retry seed persistence is preserved. Passed 3,584 terrain routes, 1,692 transition checks, 33,964 integrated AVR witness checks, and 108 gameplay/launcher checks. ATtiny85 build: 7,838 B flash, 57 B SRAM. Manual feel and enemy-inclusive route safety remain to be validated.

- **SRB preserve procedural layouts on retry:** Seeds now initialize on new game/level entry (including wrap) and survive retries; player, enemies, and lives still reset. Column generation and fixed level 1 unchanged. ATtiny85 build: 7,686 B flash (+10 B), 57 B SRAM (unchanged); all 104 existing AVR simulation checks passed. Seed lifecycle/determinism reviewed; user reported successful manual retry/progression playtesting on 2026-09-09 (see `test/test_srbpatrol/README.md`).

- **SRB safe procedural generators and flash feasibility:** Completed the [experiment](experiments/srb-generators/README.md); retained an isolated prototype and promoted terrain/progression to Todo. Passed 3,584 seeded terrain routes, 1,692 section/phase/boundary checks, and 33,964 real-AVR column/route checks. Replacement prototype: 7,828 B flash (+152 B against current baseline), 57 B SRAM (unchanged), 364 B flash free. Production gameplay unchanged. Enemy-inclusive safety/playtesting and combined enemy/retry code size remain integration checks.

- **SRB flying enemies:** Implemented metadata-selected flyers (`10`), full-byte `0b11111111` level end, vertical movement every 16 ticks, shared enemy blocking, and lives-display protection. Type/direction share slot 8 and reset on entry/reuse; level layouts unchanged. ATtiny85 build: 7,670 B flash (+174 B), 57 B SRAM (unchanged). Passed all 104 existing patrol AVR simulation checks; user manually tested flying enemies and reported good results (see `test/test_srbpatrol/README.md` for the regression checklist).

- **SRB ground enemy patrol:** Implemented left-first patrol on the existing 16-tick cadence, wall/enemy turns, terrain-only ledge landing checks, and both-edge deactivation. Directions use slot 8 and reset on level entry/slot reuse; gravity, spawning, scrolling, damage, and stomps are preserved. Passed 104 AVR instruction-simulation checks, including launcher, rendering, buttons, and restart. ATtiny85 build: 7,496 B flash, 57 B SRAM (flash +158 B, SRAM unchanged). Simulator uses an ATmega2560 instruction target; Wokwi/hardware playthrough remains unrun. See `test/test_srbpatrol/README.md`.
