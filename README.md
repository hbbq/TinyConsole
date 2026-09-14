# TinyConsole

TinyConsole is a small game console built around the ATtiny85,
with a 16×8 LED matrix display and a deliberately constrained
hardware and software platform.

The project explores how much game functionality can be built
within 8 KB of flash and 512 bytes of SRAM.

## Games

TinyConsole currently includes five apps. Each app has its own gameplay and
maintenance reference:

- [Racer](docs/Racer.md) - steer through an endless, accelerating track.
- [Breakout](docs/Breakout.md) - clear a brick wall with a paddle, ball, and
  explosive bricks.
- [SkyHop](docs/SkyHop.md) - flap through randomly positioned gaps.
- [Shift](docs/Shift.md) - rotate rows to form blinking groups and clear the
  board.
- [SRB](docs/SRB.md) - traverse scrolling levels with procedural terrain,
  enemies, and persistent progress.

## Hardware

TinyConsole uses an ATtiny85 and a 16×8 LED matrix driven by
MAX7219 controllers.

See the repository documentation and source for the current
hardware and firmware design.

## Building

The firmware is built using PlatformIO.

Individual games can be omitted to save flash by setting their `ENABLE_*`
flag to `0` in `src/appConf.h`. The launcher automatically shows only the
enabled games; at least one game must remain enabled.
