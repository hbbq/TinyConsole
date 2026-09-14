# Shift

Shift is a board-manipulation game on a 14 x 8 playfield in columns 2..15.
Columns 0 and 1 form a move indicator and row-selection marker. Every board cell
starts occupied; a sparse random subset blinks to distinguish it from the
steady cells.

## Controls

| Input | Effect |
| --- | --- |
| Up | Move the row marker toward row 0 |
| Down | Move the row marker toward row 7 |
| Action | Rotate the selected row one column left |

The selected row wraps within the 14-column board. Each shift also removes one
lit pixel from the column-0 move indicator. Exhausting that indicator currently
has no game-over effect, and the app has no score, win condition, restart prompt,
or in-game exit.

Controls are accepted on a 200 ms tick while waiting for a move. After Action,
the app waits on a 1,000 ms resolution tick before accepting more input.

## Matching, blasts, and collapse

Only blinking cells participate in matching. Orthogonally connected groups of
at least three blinking cells all match at once. A matched group produces a
blast covering every matched cell and its eight neighboring positions; the
blast is clipped at the board edges and removes both blinking and steady cells.

One resolution operation is performed after a row shift:

1. If matches exist, all matching blasts are applied.
2. Otherwise, occupied cells are compacted within each column toward row 0 if
   any can move.
3. Otherwise, nonempty columns are compacted toward the left edge if any can
   move.

The first applicable operation returns the app to input immediately. Collapse
therefore does not automatically follow a successful blast, and there is no
automatic cascade loop; a later row shift starts another resolution pass.

## Representation and randomness

The visible framebuffer holds the current on/off phase of the board. State
bytes 2..15 hold a parallel blink mask, so a blinking cell remains occupied
even while its displayed pixel is dark. Blinking XORs that mask into the
framebuffer. Matching consults the mask, while collapse treats a cell as
occupied when either representation contains it and preserves both parts when
moving it.

At board creation, every display cell is lit and each blink-mask byte is the
bitwise AND of three random bytes, giving each bit a 1/8 chance of being marked
under uniform random input. State byte 0 also holds the input/resolution phase;
the row marker and move indicator remain in the framebuffer. This reuses the
shared 16-byte state and avoids a second full board allocation on the ATtiny85.
