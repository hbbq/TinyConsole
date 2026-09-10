# SRB technical game documentation

This document describes the SRB implementation in `src/apps/SrbApp.cpp` and
`src/apps/SrbApp.h`. The current implementation is the source of truth. Historical
notes in `BACKLOG.md` explain why several mechanics exist, but do not override the
code.

## Game loop and progression

SRB is a side-scrolling, single-pixel platform game on the 16 x 8 display. The
player is always drawn at screen column 4. Moving horizontally scrolls the world
in the opposite direction and changes the player's world-space `XPOS`; the player
does not move to another screen column.

There are 16 levels:

1. Level 1 is a fixed, hand-authored level.
2. Levels 2 through 16 are generated from the level number and three byte-sized
   seeds.
3. Completing level 16 wraps to level 1. This starts a new cycle rather than
   ending the game.

At the start of every level, the level number is shown for one second. Gameplay
then starts at world x = 4 and y = 0 with zero vertical velocity, three lives, no
active enemies, and reset enemy direction/type flags. Completing a level selects
the next level, generates fresh seeds, shows its number, and enters it after the
same delay. Losing all lives restarts the current level from its beginning with
the same seeds.

Gameplay is updated on a nominal 40 ms tick:

- Buffered buttons are consumed once per gameplay tick.
- Jump input and gravity run every tick.
- Horizontal movement is allowed every fourth tick, or every 160 ms while a
  direction remains held.
- Enemy patrol/flyer movement runs when the 16-step animation counter is zero,
  once every 640 ms. Ground enemies may still fall by one row on each 40 ms
  physics tick.
- The player and enemies blink at different animation-counter intervals. The
  blinking is cosmetic; enemy occupancy is retained in state for collision.

## Controls

| Context | Input | Effect |
| --- | --- | --- |
| Gameplay | Up | Move left / scroll the world right |
| Gameplay | Down | Move right / scroll the world left |
| Gameplay | Action | Jump, but only when terrain is directly below the player |
| Saved-game prompt | Up | Continue the saved level |
| Saved-game prompt | Down | Clear saved progress and restart at level 1 |

The launcher uses its normal controls to select SRB and launch it. SRB has no
separate in-game pause or exit command.

## Column representation

World geometry is generated one vertical column at a time. A column byte maps
bit 0 to display row 0 and bit 7 to row 7. The byte is also used as a compact
content record:

- Bits 7..2 (`0xfc`) are terrain geometry.
- Bits 1..0 are metadata: `00` means no enemy, `01` means a ground patrol,
  `10` means a flyer, and `11` is reserved.
- `0xff` is the complete level-end sentinel. It must be recognized before the
  geometry and metadata fields are decoded.

Metadata is stripped before a column is inserted into the framebuffer. This
means level terrain is restricted to rows 2 through 7; rows 0 and 1 are available
for the player, enemy starts, and the lives display.

Column lookup is stateless. Scrolling backward reconstructs terrain from the
fixed level data or from the seeds. Enemy markers are processed whenever their
column scrolls in from the right, so revisiting a marker can spawn the enemy
again if one of the four enemy slots is free.

## Level structure

### Fixed level 1

Level 1 consists of 120 authored columns. The columns are stored in program
memory as two four-bit palette indices per byte, and the palette expands each
index to a geometry/metadata byte. This saves flash compared with storing every
column byte directly.

Columns 120..135 are a hardcoded `0xbc` ending structure. Column 136 and every
later lookup return the end sentinel. Because completion occurs when the next
right-edge column would be inserted, moving right completes level 1 when the
player's world x is 124.

### Procedural levels 2..16

Every procedural level uses the same broad regions:

| World columns | Purpose |
| --- | --- |
| 0..15 | Safe grounded start (`0x80`), with a row-4 landmark at 6..7 (`0x90`); no enemies |
| 16..231 | Procedurally generated terrain and patrol metadata |
| 232..238 | Ground-only exit apron (`0x80`); no enemies |
| 239..254 | Fixed `0xbc` ending structure; no enemies |
| 255 | End sentinel (`0xff`) |

The player completes a procedural level at world x = 243, when column 255 would
enter at the right side of the display.

The terrain family is selected by level number:

- Multiples of five (levels 5, 10, and 15) use bottomless platforms.
- Other even levels use independently selected holes and steps.
- Other odd levels use one selected pattern per 16-column cell.

Difficulty uses a zero-based tier:

```text
tier = min((level - 1) / 5, 3)
intensity = 5 + tier + (x / 64)
density = min(intensity, 8)
```

Integer division is used. The tier is 0 for levels 2..5, 1 for 6..10, 2 for
11..15, and 3 for level 16. `x / 64` also raises difficulty within a level at
columns 64, 128, and 192. The resulting activation threshold, out of eight, is:

| Levels | x 16..63 | x 64..127 | x 128..191 | x 192..231 |
| --- | ---: | ---: | ---: | ---: |
| 2..5 | 5 | 6 | 7 | 8 |
| 6..10 | 6 | 7 | 8 | 8 |
| 11..15 | 7 | 8 | 8 | 8 |
| 16 | 8 | 8 | 8 | 8 |

Although `density` is capped at 8, uncapped `intensity` still strengthens some
features after activation reaches 8/8: values above 8 force wide holes and a
harder odd-level pattern, and values above 9 force tall steps.

## Procedural generation

### Deterministic hash

Generation uses a small 8-bit result derived from a 16-bit input and one seed:

```text
x += seed * 31
x ^= x >> 7
x *= 13
x ^= x >> 5
result = low 8 bits
```

For grounded terrain, hashes are normally evaluated per 16-column cell
(`x >> 4`), so the feature choice remains stable across that cell. Bottomless
terrain uses 8-column cells (`x >> 3`). A separate per-column hash controls
enemy markers. These are deterministic correlated hash samples, not a stream
PRNG and not a guarantee of exact percentages in any one level.

### Even-level independent generator

Each 16-column cell starts with row-7 ground (`0x80`) and independently tests:

- A hole beginning at cell offset 4. It is one column wide normally and two
  columns wide when the cell hash requests it or `intensity > 8`.
- A step at offsets 9..10. It occupies rows 6..7 (`0xc0`) normally and rows
  5..7 (`0xe0`) when the cell hash requests it or `intensity > 9`.

The two decisions use different seeds. Offsets 0..1 also receive a row-4
landmark (`0x10` in addition to the ground), independent of feature activation.

### Odd-level pattern generator

One activation test is made per 16-column cell. If active, the second hash
selects one of four patterns:

1. A two-column hole at offsets 4..5.
2. A one-row step at offsets 9..10.
3. A row-4 platform/landmark at offsets 4..6.
4. The hole and step together.

When `intensity > 8`, selection 3 is replaced by selection 4. As on even
levels, offsets 0..1 always contain the recurring row-4 landmark above the
ground.

### Every-fifth-level bottomless generator

The world is divided into eight-column cells with no continuous row-7 floor.
Each cell contains a one-pixel-thick platform on row 5 or row 6, selected from
the first seed. The platform occupies its first six or seven columns, leaving a
two- or one-column gap. The chance of the shorter platform increases with
`density` and reaches certainty at 8/8.

### Enemy marker generation

After terrain generation, a nonempty generated column receives patrol metadata
when:

```text
(hash8(x, seed3) & 63) < 2 + tier + (x / 64)
```

Thus patrol frequency rises by tier and toward the end of a level. Empty
columns never receive a marker, and the fixed start and ending regions return
before this test. The current procedural generator only emits ground-patrol
metadata. Flyer behavior is implemented, but neither the current procedural
levels nor the fixed level-1 palette places a flyer, so flyers do not occur in
the stock level cycle today.

## Seeds and determinism

The launcher increments a 16-bit counter while it is active and uses that value
to seed TinyConsole's 16-bit xorshift PRNG when a game launches. SRB draws three
successive values in the range 0..254 whenever `startLevel(true)` is used.

Fresh seeds are generated when starting a new game, advancing to another level,
or wrapping from level 16 to level 1. Level 1 does not use them for terrain, and
advancing to level 2 replaces them again. A retry calls `startLevel(false)`, so
the three seed bytes are retained and all column lookups reproduce the same
terrain and enemy markers. Runtime enemy state is not retained: active enemies,
their directions, player position, lives, and velocity all reset.

Because columns are pure functions of `(level, seed1, seed2, seed3, x)`, the
implementation stores no level map in SRAM and can regenerate columns safely
when the player backtracks.

## Enemies

At most four enemies can be active. Each uses one packed byte: one active bit,
four screen-x bits, and three screen-y bits. Shared flag bits hold one direction
bit and one type bit per slot. New or reused slots reset both flags, so ground
patrols start moving left and flyers start moving down.

### Ground patrol

A patrol spawns at screen x = 15, y = 1, then falls one row per physics tick
until supported. Once supported, it attempts one horizontal step every 640 ms.
It reverses direction when:

- terrain or another enemy occupies the next position;
- the next column has no terrain anywhere below it, preventing deliberate
  walking into a bottomless gap.

An attempted step beyond either screen edge deactivates the patrol instead of
turning it. A patrol can step over a drop when some terrain exists below the
next position; gravity handles
the descent on following ticks. Other enemies count as blockers/support for
immediate occupancy, but the look-ahead landing search specifically requires
terrain.

### Flyer

A flyer spawns at screen x = 15, y = 3. It ignores gravity and attempts one
vertical step every 640 ms. It initially moves down. At the top/bottom edge,
terrain, or another enemy, it reverses direction without moving and waits until
the next enemy movement tick.

The engine fully supports flyer metadata even though current shipped level data
does not emit it.

### Scrolling and slot limits

When the world scrolls rightward, active enemies shift one screen column right;
when it scrolls leftward, they shift left. Enemies that would leave either edge
are deactivated. If a marker enters while all four slots are active, that spawn
is skipped. It may be attempted again if the player backtracks and later scrolls
the marker column in from the right.

## Physics, collision, and gameplay rules

Player vertical position uses four fractional bits (1/16-row units). Important
parameters are:

| Parameter | Value | Meaning |
| --- | ---: | --- |
| Normal jump velocity | -16 | 1 row/tick upward initially |
| Stomp bounce velocity | -12 | 0.75 row/tick upward initially |
| Gravity | +2 | 0.125 row/tick added each tick |
| Maximum fall velocity | +8 | 0.5 row/tick |

Terrain collision is pixel-based. Horizontal movement is rejected if terrain is
present one column to the left or right at the player's integer row. A jump can
start only when the terrain pixel immediately below the player is set. Vertical
movement stops at terrain above or below; upward position is clamped at row 0.

Enemy collision uses exact shared screen coordinates:

- Contact from horizontal movement, enemy horizontal/vertical movement, or an
  enemy falling onto the player removes that enemy and costs one life.
- If the descending player reaches an enemy during player gravity, the enemy is
  removed without damage and the player receives the smaller stomp bounce.
- There is no invulnerability period. Blinking does not change collision state.
- Enemies block each other even while their pixels are in a blink-off frame.

Falling below row 7 costs one life and returns the player to y = 0 at the current
world position. The implementation also sets the row-7 pixel in the player's
current screen column. This is the game's existing life-loss recovery block; it
can provide a landing after the player falls into a gap. It is framebuffer state,
not generated level data, so scrolling it away and later regenerating that
column restores the original terrain.

Three lives are granted on level entry or retry. Lives are shown as pixels from
right to left on row 0. Enemy rendering and blocking explicitly ignore the
lives-display cells at columns 12..15, so enemies cannot erase the UI or treat it
as terrain. When lives reach zero, the normal end-of-tick path restarts the same
level with the same procedural layout.

## Save and progress behavior

SRB reserves five EEPROM bytes:

| Byte | Contents |
| --- | --- |
| 0 | Saved level and commit byte |
| 1..3 | The three procedural seeds |
| 4 | XOR checksum of the level and three seeds |

Only levels 2..16 are valid saved levels. Saving first writes level 0 to
invalidate the record, then writes the seeds and checksum, and finally writes
the real level as the commit step. At startup, the launcher accepts the record
only when the level is in range and the checksum matches. A valid save adds a
marker to the SRB launcher icon and causes SRB to show the saved level number as
a Continue/Restart prompt.

Continue starts the saved level from its beginning using its saved seeds. It
does not restore player position, lives, active enemies, or animation state.
Restart invalidates the EEPROM record and starts level 1 with fresh seeds.

Progress is saved immediately after advancing into any level from 2 through 16.
Retries do not rewrite the save. Completing level 16 wraps to level 1 and clears
saved progress. The save is level progress, not a mid-level snapshot.

## Hardcoded and procedural content

| Hardcoded | Procedural/runtime |
| --- | --- |
| Fixed level-1 terrain and patrol markers | Levels 2..16 terrain in columns 16..231 |
| Level count and wrap at 16 | Patrol marker placement in generated terrain |
| Safe start and fixed ending regions | Three seeds for every new level entry |
| Player screen column, timing, physics constants, and three starting lives | Reconstructed columns during scrolling/backtracking |
| Enemy movement rules, spawn coordinates, and four-slot limit | Enemy positions, direction, collisions, and slot availability |
| Continue/Restart inputs and EEPROM record format | Launcher-derived initial PRNG state |

## ATtiny85 implementation constraints

SRB is shaped by the target's 8 KB flash and 512 bytes of SRAM:

- All game state fits in the shared 16-byte `console.state` array. SRB uses
  slots 0..9 and 12..15; slots 10..11 are unused.
- The four enemies are byte-packed, and all direction/type flags share one byte.
- Procedural generation replaces a stored map. Only three seed bytes and the
  current world x are needed to reconstruct terrain.
- The authored level and palette live in `PROGMEM`, not SRAM.
- Arithmetic is predominantly 8-bit. The procedural world deliberately uses the
  full byte x range and reserves 255 as its terminator.
- Player vertical motion uses one-byte signed velocity and one-byte 4.4-style
  fixed-point position rather than floating point.
- There is no dynamic allocation, object graph, per-level buffer, or persistent
  per-game global state beyond the five EEPROM bytes.

The shared state layout is intentionally compact:

| Slots | Contents |
| --- | --- |
| 0 | Animation tick or title/resume mode sentinel |
| 1 | Player world x |
| 2 | Player fixed-point y |
| 3 | Signed vertical velocity stored as one byte |
| 4..6 | Procedural seeds |
| 7 | Current level |
| 8 | Four direction bits and four flyer-type bits |
| 9 | Lives |
| 10..11 | Unused by SRB |
| 12..15 | Four packed enemies |

The most recent historical result recorded in the `BACKLOG.md` Done section for
the integrated, intensity-tuned generator was 7,906 bytes of flash and 57 bytes
of static SRAM, leaving 286 bytes of flash at that revision. That number was not
re-measured for this documentation-only change, but it illustrates why future
mechanics should be evaluated for both flash and SRAM cost.

## Evolution and verification context

The Done history records the main design sequence: ground enemies gained patrol
and ledge-aware behavior; flyer support was added using shared packed state;
retry seeds were made persistent; the old procedural terrain was replaced with
three reachability-checked families and a 16-level cycle; and terrain intensity
was then raised to improve pacing and scrolling readability. Those changes
explain the current deterministic retry policy, recurring landmarks, conservative
gap/step bounds, and compact enemy representation.

The generator's historical route checks establish terrain-only paths without
requiring damage, recovery blocks, or stomps. They do not prove that every live
enemy arrangement is damage-free. The patrol regression harness under
`test/test_srbpatrol/` covers the real game/launcher/framebuffer logic in an AVR
instruction simulator; display electronics and subjective playability still
require Wokwi or hardware testing.
