---
name: implement
description: Implement a ready Todo item from BACKLOG.md, verify it, and update the backlog.
---

# Implement

Implement a Todo item that is already sufficiently understood and ready for implementation.

Before making changes:

1. Read `AGENTS.md`.
2. Read `BACKLOG.md`.
3. Identify the requested Todo item.
4. Inspect the relevant existing code before deciding how to implement it.

## Workflow

### 1. Validate the Todo

Confirm that the item is sufficiently defined to implement without major unresolved design decisions.

If important decisions are still missing:
- Do not guess unnecessarily.
- Explain what is unresolved.
- Leave the item in Todo unless a small, obvious decision can safely be made from existing project conventions.

### 2. Start implementation

Move the item from `Todo` to `In Progress`.

Preserve any relevant constraints or decisions recorded with the item.

### 3. Implement

Recommended execution mode: DEV by default.
Use THINK when the TODO requires substantial design or investigation.
Suggest HARD only for unusually difficult tasks.

- Make the smallest sensible change that satisfies the Todo.
- Follow existing project patterns.
- Follow all constraints in `AGENTS.md`.
- Avoid unrelated refactoring.
- Do not introduce abstractions unless they solve a concrete need for the current item.
- Do not add tests solely to satisfy the workflow. Add automated tests only when they provide clear value for the change or are explicitly requested.

### 4. Verify

Always run the relevant build described in `AGENTS.md`.

For TinyConsole:
- Run `pio run -e attiny85`.
- Report flash and SRAM usage.
- Fix compilation or build failures caused by the implementation.
- Do not create automated tests or simulator test infrastructure unless explicitly requested.
- Behavioral and gameplay verification may be left for manual testing when that is simpler and more appropriate.

If manual verification is required, clearly state what should be tested.

### 5. Finish

When implementation and verification succeed:

- Move the item from `In Progress` to `Done`.
- Add a short outcome if it records useful implementation information.
- Keep the Done entry concise.

## Final response

Summarize:

- what was changed,
- how it was verified,
- resulting flash/RAM usage when applicable,
- any remaining limitations or follow-up work.

Do not create new backlog items unless they represent genuinely separate follow-up work.