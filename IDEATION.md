# TinyConsole Ideation Guidance

Generate genuinely interesting ideas for TinyConsole.

The purpose of ideation is to discover small games, features, technical experiments, or unusual uses of the platform that fit its intentionally constrained character.

Prefer ideas that make the limitations part of the design rather than trying to hide them.

## Platform character

TinyConsole is deliberately tiny and constrained.

Typical characteristics include:

- ATtiny85
- 8 KB flash
- 512 bytes SRAM
- 16x8 monochrome LED display using MAX7219
- Very limited input controls
- Simple sound where available
- EEPROM is available for small amounts of persistent state
- Firmware size and RAM usage matter

Some variants or future versions may differ, but ideas should normally fit the current TinyConsole design unless the repository documentation explicitly says otherwise.

## Prefer

Favor ideas in areas such as:

- Small games with mechanics that work naturally on a 16x8 display
- Unusual game mechanics that emerge from the hardware constraints
- Procedural or semi-procedural level generation
- Games where very little state creates surprisingly rich behavior
- Creative uses of blinking, animation, timing, scrolling, or limited pixel states
- Clever use of EEPROM for progress, unlocks, seeds, scores, or persistent world state
- Replayable games that require little stored level data
- Generative visual experiments
- Tiny simulations
- Technical demonstrations that expose interesting properties of the hardware
- Sound or rhythm experiments that remain feasible on the platform
- Ideas that reuse existing TinyConsole capabilities in unexpected ways
- Features that improve the character or identity of TinyConsole rather than merely making it more conventional

Ideas may be playful, strange, unnecessary, experimental, or slightly absurd if they are technically plausible and interesting.

## Especially interesting

Ideas are particularly valuable when they:

- Turn a limitation into a gameplay mechanic
- Allow large or varied experiences from very little code or data
- Produce emergent behavior from simple rules
- Make good use of deterministic randomness or compact procedural algorithms
- Could plausibly fit alongside existing TinyConsole applications
- Demonstrate something that feels surprising given the hardware

## Avoid

Do not propose ideas whose main value is:

- Generic refactoring
- Architectural cleanup
- Adding abstraction layers without a concrete user-visible benefit
- Increasing test coverage for its own sake
- Making the project resemble a larger modern software platform
- Adding infrastructure that is disproportionate to the feature
- Replacing a simple existing solution with a more elaborate one
- Features that obviously require substantially more capable hardware
- High-resolution graphics concepts that do not translate meaningfully to 16x8 pixels
- Network or cloud features unless the repository documentation explicitly describes hardware that supports them

Do not propose ideas already represented by open issues.

Avoid minor variations of existing ideas unless the variation introduces a genuinely new mechanic or technical approach.

## Scope

Prefer ideas that could reasonably become one GitHub issue.

Small and medium ideas are usually better than large redesigns.

A good idea should be understandable independently and should leave room for later triage and investigation to determine the exact implementation.

If no idea feels genuinely worthwhile, return no idea rather than inventing filler.
