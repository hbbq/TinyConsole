# Breakout

Move the paddle with Up and Down and keep the ball in play while clearing the
brick wall. Missing the ball advances the wall one column toward the paddle.

One brick at a time is explosive. It blinks to distinguish it from normal
bricks, but remains solid while its pixel is dark. Hitting it destroys the
center brick and every brick in the surrounding 3x3 area. The blast is clipped
at the display edges and never clears the paddle column. Explosions do not
chain-react.

The first explosive brick has a fixed position. After it explodes, a replacement
appears in the rightmost column, four rows away from the previous one. The
marker moves left with the wall; if it reaches the paddle edge, it moves to the
newly inserted rightmost column.
