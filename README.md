# TinyConsole

TinyConsole is a small game console built around the ATtiny85,
with a 16×8 LED matrix display and a deliberately constrained
hardware and software platform.

The project explores how much game functionality can be built
within 8 KB of flash and 512 bytes of SRAM.

## Games

### SRB

A scrolling platform game with level progression, procedural
generation, enemies and persistent progress.

See [SRB documentation](docs/SRB.md) for gameplay mechanics,
level generation and implementation details.

## Hardware

TinyConsole uses an ATtiny85 and a 16×8 LED matrix driven by
MAX7219 controllers.

See the repository documentation and source for the current
hardware and firmware design.

## Building

The firmware is built using PlatformIO.