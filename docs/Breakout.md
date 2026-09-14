# Breakout

Breakout is a paddle-and-ball game played across the 16 x 8 display. The
seven-pixel paddle occupies column 0, the ball starts beside it, and columns
8..15 begin as a solid brick wall. The game advances every 100 ms.

## Controls and game flow

| Input | Effect |
| --- | --- |
| Up | Move the paddle up one row |
| Down | Move the paddle down one row |
| Action | No in-game action |

The paddle is clamped to the display. When it returns the ball, the horizontal
direction reverses and a random vertical velocity is chosen. The ball also
bounces from the top, bottom, and right edges. Brick collision accounts for the
horizontal and vertical parts of a diagonal move so that edge and corner hits
choose an appropriate bounce direction.

Missing the ball advances the wall one column toward the paddle, inserts a full
new column at the right edge, and relaunches the ball horizontally from its
current position. There is no separate lives counter, score, win screen, or
automatic restart after all original bricks have been cleared.

## Explosive brick

One brick at a time is explosive. It blinks to distinguish it from normal
bricks, but remains solid while its pixel is dark. Hitting it destroys the
center brick and every brick in the surrounding 3x3 area. The blast is clipped
at the display edges and never clears the paddle column. Explosions do not
chain-react.

The first explosive brick has a fixed position. After it explodes, a replacement
appears in the rightmost column, four rows away from the previous one. The
marker moves left with the wall; if it reaches the paddle edge, it moves to the
newly inserted rightmost column.

## Implementation notes

The ball's vertical position and velocity use 1/16-row fixed-point units, while
its x position is an integer column. The ball x position, paddle center, and
horizontal direction share one packed state byte; vertical position, vertical
velocity, and the packed explosive-brick coordinate use three more. Brick
occupancy lives in the framebuffer, so no separate board is stored in SRAM.

Randomness is used only to choose the vertical velocity after a paddle return.
The launcher seeds the shared generator when the app starts. Launching the app
always rebuilds the initial wall, ball, paddle, and explosive brick.
