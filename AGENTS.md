# Repository Guidelines

## Project Structure & Module Organization

- `src/main.cpp` implements the game launcher and Arduino `setup()`/`loop()`.
- `src/apps/` contains games, `Game` dispatch, `GameFactory`, and icon data. Follow the existing `NameApp.h`/`NameApp.cpp` pattern for new games.
- `src/hardware/` contains the console driver and shared game API; `src/text/` provides number rendering. `src/TinyRandom.*` supplies random utilities.
- `chips/` holds the custom Wokwi button chip source, definition, and compiled WebAssembly. `diagram.json` defines simulator wiring; `wokwi.toml` selects firmware artifacts.
- `include/`, `lib/`, and `test/` contain shared headers, private libraries, and test/support code.

## Build, Test, and Development Commands

Run commands from the repository root with PlatformIO installed:

- `pio run -e attiny85`: compile the Arduino firmware and report flash/RAM usage. This is the normal required verification for implementation changes.
- `pio run -e attiny85 -t clean`: remove generated build output before rebuilding.
- `pio run -e attiny85 -t upload`: upload to hardware after configuring a compatible programmer and connection.

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

## Documentation

Game-specific technical documentation lives under `docs/`. When an implementation change affects documented gameplay behavior, update the relevant documentation in the same change. The current implementation is the source of truth. The `BACKLOG.md` Done section may provide historical context, but should not normally be required for future implementation tasks.

## Testing Guidelines

For normal development and agent-driven implementation, verify changes with `pio run -e attiny85` and report flash/RAM usage.

Do not run the AVR-GDB regression suite under `test/test_srbpatrol` (including `test/test_srbpatrol/run.ps1`) unless the user explicitly requests it. The suite is slow and is not part of the normal implementation or verification workflow.

Interactive gameplay, rendering, controls, and other behavior that benefits from end-to-end checking may be verified manually in Wokwi or on physical hardware when appropriate. There is no coverage threshold or requirement to run the AVR-GDB suite before opening a PR.

## Commit & Pull Request Guidelines

History contains only `Initial commit`, so no established message convention exists. Use concise, imperative subjects, such as `Fix launcher button debounce`. Keep commits focused. PRs should explain behavior changes, link relevant issues, record build and simulation/hardware checks, and include screenshots for visible changes. Explain intentional changes to generated `firmware.map` or chip binaries.

## Codex execution modes

Use these names when recommending how a task should be executed:

- QUICK — Terra / low
  Simple, fast, localized code changes requiring little analysis.

- DEV — Sol / medium
  Implement well-specified TODOs and normal development work.
  A TODO may be technically complex and still be DEV if the intended
  behavior and approach are sufficiently specified.

- THINK — Astra / medium
  Use when substantial reasoning is needed to determine what should be
  built or how it should work: exploring ideas, investigation, evaluating
  alternatives, architecture, or turning unclear requirements into
  well-specified TODOs.

  Do not recommend THINK merely because an implementation is large or
  technically complex.

- HARD — Astra / high
  Difficult debugging or unusually demanding tasks where deeper reasoning
  is worthwhile, such as aggressively optimizing RAM/flash usage.

When appropriate, state a recommended execution mode before starting work.
Do not automatically prefer a higher mode; use the lowest mode appropriate
for the task.

Do not recommend a higher execution mode merely as a precaution.
Recommend escalation only when the task actually requires the kind of
reasoning described by the higher mode.
