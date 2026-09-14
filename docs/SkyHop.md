# SkyHop

SkyHop is an endless gap-navigation game. The player remains at column 2 while
single-column obstacles move from right to left. Each obstacle has a randomly
placed four-pixel opening, and obstacle columns are separated by seven empty
columns.

## Controls and game flow

| Context | Input | Effect |
| --- | --- | --- |
| During a run | Action | Set the player's velocity upward |
| Score display | Action | Start a new run |
| Any | Up / Down | No in-game action |

The game updates every 125 ms. Holding Action reapplies the upward velocity on
each update; on updates without Action, gravity increases downward velocity to
a fixed maximum. Touching an obstacle or leaving the top or bottom of the
display ends the run and shows the score. A restart clears the screen, score,
obstacle phase, position, and velocity.

## Obstacles and scoring

The first obstacle is generated on the first gameplay update. Thereafter, the
framebuffer shifts left every update and a new obstacle is generated every
eighth column. Its gap position is selected uniformly from the five placements
that fit on the display.

The score increases, up to 255, when an obstacle column reaches the player's x
coordinate. The player pixel is removed before this occupancy check, so it is
not counted as an obstacle. The obstacle movement happens before collision is
tested at the player's new vertical position.

## ATtiny85 implementation notes

Vertical position uses 1/16-row fixed-point units and signed one-byte velocity.
Only four state bytes are used: position, velocity, a combined game-over/spacing
byte, and score. Obstacles live entirely in the framebuffer. The launcher seeds
the shared random generator once when the app starts; restarting a run continues
the same random sequence.
