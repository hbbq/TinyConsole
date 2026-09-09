# Repository Guidelines

## Project Structure & Module Organization

- `src/main.cpp` implements the game launcher and Arduino `setup()`/`loop()`.
- `src/apps/` contains games, `Game` dispatch, `GameFactory`, and icon data. Follow the existing `NameApp.h`/`NameApp.cpp` pattern for new games.
- `src/hardware/` contains the console driver and shared game API; `src/text/` provides number rendering. `src/TinyRandom.*` supplies random utilities.
- `chips/` holds the custom Wokwi button chip source, definition, and compiled WebAssembly. `diagram.json` defines simulator wiring; `wokwi.toml` selects firmware artifacts.
- `include/`, `lib/`, and `test/` currently contain placeholder documentation for shared headers, private libraries, and tests.

## Build, Test, and Development Commands

Run commands from the repository root with PlatformIO installed:

- `pio run -e attiny85`: compile the Arduino firmware and report flash/RAM usage.
- `pio run -e attiny85 -t clean`: remove generated build output before rebuilding.
- `pio run -e attiny85 -t upload`: upload to hardware after configuring a compatible programmer and connection.
- `pio test -e attiny85`: run PlatformIO tests once test suites exist; currently there are none.

For local simulation, build first, then launch the project through the Wokwi VS Code extension. Configuration points to `.pio/build/attiny85/firmware.hex` and `firmware.elf`.

## Coding Style & Naming Conventions

Match surrounding formatting: most app and hardware code uses four spaces, while `main.cpp` and the custom chip use two. Keep opening braces on the declaration line. Use PascalCase for classes, camelCase for methods and variables, and existing uppercase patterns for button masks and state slots. Headers use `#pragma once`. Prefer fixed-width integers and compact state appropriate to the ATtiny85. No formatter or linter configuration is checked in.

## Project Constraints

TinyConsole targets the ATtiny85 with 8 KB flash and 512 bytes SRAM.

Resource usage is a primary design constraint:

- Prefer solutions that minimize both flash and SRAM usage.
- Avoid persistent per-game global state where possible.
- Do not trade significant SRAM usage for small flash savings without a clear reason.
- After implementation changes, run `pio run -e attiny85` and report flash and RAM usage.
- Prefer simple implementations appropriate for the constrained target over abstractions that increase code size.

## Implementation Philosophy

Keep implementations small and pragmatic.

Do not introduce abstractions, frameworks, or generalized infrastructure unless they solve a concrete current need. Prefer existing project patterns over architectural improvements for their own sake.

## Testing Guidelines

No test framework is explicitly configured and no coverage threshold exists. Place future suites under `test/test_<feature>/`. For changes, build and exercise affected games in simulation or on hardware, checking launcher navigation, button handling, rendering, and restart behavior. Review memory usage after firmware changes.

## Commit & Pull Request Guidelines

History contains only `Initial commit`, so no established message convention exists. Use concise, imperative subjects, such as `Fix launcher button debounce`. Keep commits focused. PRs should explain behavior changes, link relevant issues, record build and simulation/hardware checks, and include screenshots for visible changes. Explain intentional changes to generated `firmware.map` or chip binaries.
