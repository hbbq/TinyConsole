# Racer

Racer is an endless obstacle-avoidance game. The player is a single pixel in
column 0 and moves vertically while a randomly curving track scrolls from right
to left. Lit pixels outside the track are walls; the unlit band is the drivable
area.

## Controls and game flow

| Input | Effect |
| --- | --- |
| Up | Move the player up one row |
| Down | Move the player down one row |
| Action | No in-game action |

Input, collision, and one track step are processed on the current speed
interval. The player is clamped to the eight display rows. A collision with the
approaching wall clears the display and immediately starts a new run; there is
no score or game-over prompt.

## Track generation and difficulty

Each update shifts the framebuffer left and constructs one new wall column.
The track may move up, move down, or continue straight. An immediate reversal
from up to down (or down to up) is suppressed, and the track is kept within the
display.

Difficulty is derived from a 16-bit step counter, which saturates at 60,000:

- the update interval falls from 500 ms toward a minimum of 50 ms;
- the track narrows as the run continues, becoming two pixels wide after 2,000
  steps;
- the random curve parameter decreases, making direction changes more likely.

The launcher seeds the shared random generator at app start. A crash resets the
player, track position, step counter, speed, width, and previous curve direction,
but continues from the generator's current random state.

## ATtiny85 implementation notes

The framebuffer is the complete visible track history. Six state bytes hold the
packed player/track position, packed width/curve parameter, 16-bit step count,
speed, and previous movement. Speed is stored in 25 ms units to fit in one byte;
there is no separately allocated course or score buffer.
