# SRB generator experiment — 2026-09-09

Result: three deterministic terrain generators plus 16-level progression fit
without new persistent state. Keep this prototype outside production until the
terrain/progression Todo is implemented. No playable source was changed.

## Movement bounds and safe rules

The search models `SrbApp::doJump`, `doGravity`, and `move` in update order:
40 ms physics, horizontal motion once per four ticks, jump velocity -16,
gravity +2, fall cap +8, and 1/16-row positions. It rejects every fall below
row 7; it never uses life-loss recovery blocks or enemy bounces. A grounded
jump rises 56/16 = 3.5 rows. Row rounding permits climbing a four-row wall
in favorable isolated geometry; this is not a recommended generator limit.

| Feature | Isolated search result | Prototype bound |
| --- | --- | --- |
| Equal-height gap | Widths 1–4 pass; 5–6 fail | 1–2 empty columns |
| Solid step | Heights 1–4 pass; 5 fails | Height 1–2, width 2 |
| Raised landing from ground | Surface rows 3–5 pass; row 2 fails | Row 4, width 2–3 |
| Bottomless landing | All selected height/gap transitions pass | Surface row 5 or 6; width 6–7; gap 1–2; rise/drop at most 1 |

Search allows arbitrary waits and jump timing, with forward motion only.
Failures are not general impossibility proofs for arbitrary backtracking or
other terrain. Passing bounds do not mean a layout is forgiving to play.

Grounded generators use 16-column cells with these zero-based offsets:

- Independent features: a hole at 4 (width 1–2), a step at 9–10
  (height 1–2), and an optional row-4 platform at 0–1. Each decision uses
  its own seed. Reserved spaces prevent ceilings overlapping gap/step jumps.
- Pattern features: choose a two-column hole at 4–5, a one-row step at
  9–10, a row-4 platform at 4–6, or the hole plus step. Do not combine
  the pattern platform with other features.
- Bottomless: 8-column cells, with row-5/6 platforms occupying the first
  six or seven columns. The trailing one or two columns are empty.

There are settled checkpoints at grounded cell offset 2 and bottomless cell
offset 2. Adjacent-cell checks start and end at these checkpoints, cover all
four horizontal tick phases, and include a superset of the generated feature
combinations. This lets successful cell crossings compose without requiring
an airborne arrival, damage, or enemy support. Both fixed boundary transitions
are checked separately. Do not move features or widen gaps without rechecking.

Columns 0–15 are ground only. Columns 232–238 are a ground-only exit apron,
so the last cell is truncated safely. The required ending is unchanged:
238 = `0x80`, 239–254 = `0xbc`, 255 = `0xff`. Row 6 is a passage below
the ending's row-2–5 blocks; the game triggers completion when column 255
would scroll in (player world x = 243), not when the player reaches x = 255.

## Initial pacing

`tier = min((level - 1) / 5, 3)`, using integer division. Preserve fixed
level 1. Every fifth level is bottomless; other evens use independent
features, other odds use patterns. Set level count to 16 initially and wrap.

Activation threshold out of eight:

| Tier / levels | x 16–63 | x 64–127 | x 128–191 | x 192–231 |
| --- | --- | --- | --- | --- |
| 1 / 1–5 | 2 | 3 | 4 | 5 |
| 2 / 6–10 | 3 | 4 | 5 | 6 |
| 3 / 11–15 | 4 | 5 | 6 | 7 |
| 4 / 16+ | 5 | 6 | 7 | 8 |

Use this threshold for each independent feature or whole pattern. On
bottomless levels it selects two-column gaps instead of one-column gaps.
The prototype's initial enemy threshold is the same number out of 64 per
nonempty column (nominal 3.125–12.5%), with no spawns in the start or exit
apron. Hash decisions are correlated deterministic samples, not a guarantee
of exact per-level percentages. Enemy type/eligibility and retry persistence
remain the separate existing Todos.

## Verification and resource results

- 3,584 complete terrain routes: 256 seed triples for each of levels
  2, 3, 5, 6, 7, 10, 11, 12, 15, 16, 17, 20, 254, 255. Seeds are
  `(s, 73*s+19, 151*s+97) mod 256`, covering each byte value in each slot.
  This samples the seed space; it does not enumerate all 256 cubed triples.
- 1,692 cell/phase and fixed-boundary checks, including all 19 grounded
  variants paired with each other, and both bottomless heights/gap widths.
- Forward/reverse column lookup agrees; every sampled start/end is exact.
- 14 sample routes replayed through the real AVR `doJump`, `doGravity`,
  and `move`: **33,964 checks passed**. C++ columns match the host model
  byte-for-byte in both directions; trajectory checksums and completion
  match. Uses the existing AVR instruction simulator approach, with an
  ATmega2560 instruction target to avoid ATtiny relative-call-wrap limitations.

| Build | Flash | Static SRAM | Flash remaining |
| --- | --- | --- | --- |
| Current working-tree baseline | 7,676 B | 57 B | 516 B |
| Replacement prototype + 16-level count | 7,828 B | 57 B | 364 B |
| Difference | +152 B | 0 B | -152 B |

Both builds use `pio run -e attiny85`, the same project options and installed
AVR compiler. The older backlog baseline was 7,670 B; the current uncommitted
source builds six bytes larger. Prototype replaces old generation helpers
and the procedural body rather than retaining both. It adds no persistent
globals. SRAM figures are linker static allocations, not peak stack usage.
The 364 B remaining must also accommodate the separate enemy/retry Todos;
their combined final size is not established by this experiment.

The replay disables enemies between ticks. It proves terrain can be crossed
without damage/recovery or stomps for support, **not** that every live enemy
arrangement can be avoided. Enemy-inclusive damage-free play, feel, rendering,
launcher navigation and retry behavior have not been playtested here. Validate
these when integrating the ready Todos; do not claim full gameplay safety
from the terrain search alone.

## Reproduce

From the repository root, with Node and the existing PlatformIO installation:

```powershell
node experiments/srb-generators/prepare.cjs
node experiments/srb-generators/check.cjs 256
powershell -NoProfile -ExecutionPolicy Bypass -File experiments/srb-generators/replay.ps1
& "$env:USERPROFILE/.platformio/penv/Scripts/pio.exe" run -e attiny85 -d .pio/srb-experiment/build
& "$env:USERPROFILE/.platformio/penv/Scripts/pio.exe" run -e attiny85
```

`prepare.cjs` copies current source/config into ignored `.pio/srb-experiment/`
and substitutes `generator.h`. The host model is in `check.cjs`; generated
route witnesses, results, AVR replay source, firmware and its map stay in that
ignored directory. The production source and map are never replaced by the
prototype. No new dependencies or general simulator infrastructure are needed.
