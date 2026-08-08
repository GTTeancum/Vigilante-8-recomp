# TO-DO 9 — pause menu layout in widescreen

Reported with a screenshot: in 16:9 the pause menu's selection border and its
Track / Quit / Resume entries are offset from each other and overlap. Also
noted, and separate: neither the menu nor the border is centred in 16:9.

## Reproduction

`tools/recompone-v8-2/run_pause_probe.py` taps START a little way into
gameplay and retains a burst of presentation frames, so the paused screen can
be captured at either aspect. `RECOMPONE_TRACE_MODAL_RECTS=1` logs the exact
geometry of every rectangle and triangle drawn while the modal is up.

4:3 is correct: one centred border with the caption and both rows inside it.

## Cause of the misalignment

Widescreen HUD anchoring (`EnhancedGlBackend.DrawRect`) shifts a rectangle left
by the widescreen margin when it sits in the top 42% of the screen or the left
third. That is right for the HUD, which is authored against the screen edges,
and wrong for a centred modal. The pause menu straddles the test: the caption
and entries sit high enough to be anchored and move, while the border is drawn
lower **and as triangles rather than rectangles**, so it never reaches that
code at all. The panel tears in half.

Measured in target space, at either aspect:

```
panel rectangle   x  64..256   centre 160
border triangles  x  48..276   centre 162   (16:9)
                  x -21..341   centre 160   (4:3)
```

Both are authored on the same centre, so the modal needs no shift to line up --
it needs to be left alone. A first attempt applied the anchor uniformly to the
whole modal instead of exempting it; that made the entries consistent with each
other but pushed the group one margin left of the border, which the geometry
above explains.

## Fix

`GpuHle.NativeModalActive` is raised for the frames in which the retail
PAUSED / QUEST OBJECTIVES / ARE YOU SURE? overlay is drawn, and HUD anchoring
is skipped while it is set.

It is a short hold rather than a single flag. The game builds a frame's packets
during logic and the backend draws them after the next frame boundary, so a
flag set by the text hook and cleared at that boundary is always false by the
time the rectangles are emitted -- the first version of this fix changed
nothing at all for exactly that reason.

Verified headlessly at both aspects: 16:9 now matches the 4:3 structure, with
the caption and both rows inside the border, and 4:3 is unchanged (margin is
zero there, so the anchoring path was already inert).
`artifacts/pause/pause_menu_fix.png`.

## Centring the modal

Exempting the modal left it internally consistent but sitting +12.6% of width
right of screen centre -- the widescreen margin is 54 against a 428-wide frame,
12.6% exactly, so the whole thing was displaced by precisely one margin.

The border is drawn as triangles, so the question was how to move it without
touching the world still being rendered behind the pause (~3900 triangles a
frame, against ~38 for the menu). `PrimFlags` has no UI marker and the
ordering-table index is a 2654..4096 continuum shared with the scene, so
neither separates them. What does: the modal's border comes straight from a
packet and never touches the GTE, so its vertices carry neither view space nor
a GTE Z, while every world triangle carries both. That test picks out exactly
the ~38 flat 2D triangles.

Which primitive to move was then measured rather than guessed, using the green
selector arrows as a landmark and normalising by border width:

| shift (rect, tri) | border off-centre | content vs border |
|-------------------|-------------------|-------------------|
| 4:3 reference     | -0.5 px           | +10.8%            |
| (0, 0)            | +161 px           | -13.6%            |
| (-margin, -margin)| +3 px             | -14.2%            |
| **(0, -margin)**  | **+3 px**         | **+10.4%**        |

Moving both together centred the border but carried the content with it, so
the relative error never changed. Only the border needed correcting. The
rectangles were already right once HUD anchoring stopped moving them per
piece.

## Result

16:9 reproduces the 4:3 layout to within half a percent of border width, with
the border centred. 4:3 is unchanged -- margin is zero there, so both the
anchoring path and the triangle shift are inert.
`artifacts/pause/pause_menu_fix.png`.
