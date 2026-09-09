---
name: backlog
description: Review, refine, add, move, and prioritize items in the project backlog.
---

# Backlog

Use `BACKLOG.md` as the project's lightweight planning system.

Before working with the backlog:

1. Read `BACKLOG.md`.
2. Read `AGENTS.md`.
3. Inspect relevant code when necessary to understand an item.

## Workflow

Backlog items move through:

Ideas → Experiments or Todo → In Progress → Done

Not every idea requires an experiment.

### Ideas

Use Ideas for things worth remembering but not yet sufficiently understood or decided.

When discussing an idea:
- Clarify the goal.
- Identify important unknowns.
- Keep exploration proportional to the size of the idea.
- Do not turn an early idea into an implementation plan prematurely.
- Recommended execution mode: THINK

An idea may remain an Idea, move to Experiments, move directly to Todo, or be removed if it is no longer useful.

### Experiments

Use Experiments when a small investigation or prototype can answer an important question before implementation.

Record what the experiment is intended to learn.

After the experiment, record the result and decide whether the item should move to Todo, return to Ideas, or be discarded.

### Todo

Todo items should be sufficiently understood to implement without major design decisions.

Keep the description concise, but preserve constraints and decisions that will matter during implementation.

### In Progress

Move an item here when implementation actually begins.

### Done

Move completed work here.

Keep a short outcome when it contains useful information that would otherwise be lost.

## General rules

- Keep `BACKLOG.md` concise.
- Do not create unnecessary process or documentation.
- Preserve useful reasoning and decisions, not conversation history.
- Do not implement backlog items unless explicitly asked.
- When asked what to work on, explain the most useful candidates rather than automatically choosing one.