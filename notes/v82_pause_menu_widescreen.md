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

## Still open: the menu is not centred in 16:9

The border and its contents now move together, but the group sits right of
screen centre by +12.6% of width. The widescreen margin is 54 against a
428-wide frame -- 12.6% exactly -- so the whole modal is displaced by precisely
one margin.

This is not fixed here, and deliberately so. Correcting it means shifting the
border, which is drawn as triangles, and there is no reliable way to tell the
modal's triangles from world geometry: `PrimFlags` has no UI marker, and during
the modal the ordering-table index is a 2654..4096 continuum shared with the
scene behind it. Keying on that would risk shifting world geometry to fix a
menu. It belongs with widescreen support (item 1), where the 2D overlay origin
can be settled once for every element rather than guessed at per screen.
