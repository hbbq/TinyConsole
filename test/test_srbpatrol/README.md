# SRB patrol regression checks

From the repository root on Windows, with the project's PlatformIO packages installed:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File test/test_srbpatrol/run.ps1
```

The runner compiles the real game, launcher, and framebuffer code and runs it
under `avr-gdb`'s instruction simulator. Only Arduino time and ADC input are
stubbed. It checks wall/enemy turns, independent direction bits, both screen
exits, ledge descent, bottomless gaps, enemy support and falling, scrolling,
spawn-slot reuse, damage, stomps, 16-tick cadence, framebuffer rendering,
buttons, restart, level transitions, and launcher navigation into SRB.

The simulator does not emulate the ATtiny85's 8 KB relative-call wrap, so the
test executable uses the compatible ATmega2560 instruction target. Run
`pio run -e attiny85` separately for production target and memory verification.
These checks do not emulate the display electronics or replace a Wokwi/hardware
playthrough. Test artifacts go under the ignored `.pio/test_srbpatrol/` directory.

## Retry layouts: manual regression checklist

The existing 104 checks cover restart resets and level transitions. Seed lifetime
and deterministic column lookup were reviewed in code. The user reported successful
manual retry/progression playtesting on 2026-09-09. Retain this checklist for future
Wokwi/hardware regression checks:

- On procedural level 2, note terrain and enemy spawn locations, scroll backward
  and forward, then lose all lives. Confirm the same layout after retry, with
  three lives, the player at the start, and live enemies reset.
- Complete level 2 to wrap to fixed level 1, then reach level 2 again. Confirm a
  fresh procedural layout. With a debugger, check seed slots 4..6 initialize on
  each progression, including wrap, and stay unchanged on retry.
- Restart the console and launch SRB again. Confirm seeds initialize for the new
  game and fixed level 1 remains unchanged; check controls and rendering.

## Flying enemies: manual regression checklist

The 104 automated checks cover the existing patrol behavior, not flyers.
The user manually tested flying enemies and reported good results on 2026-09-09.
For a Wokwi/hardware check, temporarily use metadata `10` in a level column
(for example, `0b00000010` for a terrain-free flying spawn). Restore the level
after testing; deliberate flyer placement is separate backlog work.

- Check row-1 spawning, initial downward motion, no gravity, and one vertical
  pixel per 16 ticks. At rows 0 and 7, terrain, or another enemy, the flyer
  must turn in place and wait until the next movement tick.
- Check flying/flying and flying/ground blocking in both movement directions,
  including a ground enemy falling onto a flyer. Reuse each of the four slots
  with both types and check that direction and type reset independently.
- With lives 0 through 4, check enemies at row 0, columns 12 through 15,
  through draw/erase, blinking, and scrolling in both directions. The life
  count must stay unchanged, and movement must ignore life pixels while still
  blocking on enemy state. Zero lives should trigger the normal restart.
- Check flyer contact damage and stomps, scrolling off either edge, restart,
  and both level transitions. Only `0b11111111` ends a level; metadata `11`
  in any other column is reserved and must not spawn an enemy.
