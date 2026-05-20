# Renderer Functions (OUT OF SCOPE)

109 functions identified in the main EXE that belong to the renderer
have been removed from `src/{assets,gameplay,physics}/auto/`. They are
explicitly out of scope per `CLAUDE.md` -- the renderer will be
rewritten and the new layer can present whatever interfaces it likes.

The functions are grouped by purpose. Pass 2 may further re-categorize
some as "render-physics seam" (e.g. anything that *reads* Vehicle state
to build a draw packet sits on the contract surface).

## VRAM bin packer (guillotine)

```
0x80017d5c  Vram_BinPacker_Init        free the tree, reset
0x80017db4  Vram_BinPacker_Destroy
0x80017e0c  Vram_BinPacker_Reset
0x80017e3c  Vram_BinPacker_NewLeaf     (x,y,w,h) -> 0x18-byte node
0x80017ec4  Vram_BinPacker_SplitX      vertical split  (mode=2)
0x80017f4c  Vram_BinPacker_SplitY      horizontal split (mode=3)
0x80017fd4  Vram_BinPacker_InitFor640  initial split 0x140 + 0x1e0 + 0x100
```

Node struct (0x18 bytes):
- +0x00 i16 x, +0x02 i16 y, +0x04 i16 w, +0x06 i16 h
- +0x08 i16 mode (0=leaf, 2=vsplit, 3=hsplit)
- +0x0a i16 reserved
- +0x10 ptr leftChild
- +0x14 ptr rightChild

## Font / text rendering

```
0x80019034  Font_AllocAtlas
0x80019138  Font_MeasureText
0x80019960  Font_DrawText          (handles '\n' line break and 0x01 color escape)
0x80019d10  Font_DrawWrappingBlock (with scroll counter param)
0x80019c64  Font_AppendOTagPrim
0x80019f44  Font_DrawSpanRect
0x800197f4  Font_AllocDrawPrim
0x80019370  Font_BuildChar         (build a single sprite prim for char)
0x80019458  Font_LayoutWord
0x800190a8  Font_FreeAtlas
0x80019010  Font_SetColor
0x80019f9c  Font_SetTint
0x8001a0ac  Layout_AlignNext       (x,y align helper used by splash too)
0x8001a91c  Layout_BeginCenter
0x8001a994  Layout_EndCenter
0x8001aa0c  Layout_PushClip
0x8001aa38  Layout_PopClip
0x800187e4  Font_DecodeFNT         (the .FNT file decoder)
0x8001884c  Font_BuildTexture      (uploads glyph atlas via LoadImage)
... and ~80 more renderer helpers in the same source files.
```

## Draw-OT submit

```
0x80012828  Render_SubmitDrawOTag  (sets up DrawSyncCallback then DrawOTag)
```

## Per-vehicle render seam

```
0x8001db24  Render_VehicleSetTrans (Vehicle::pos -> SetTransMatrix)
0x8001d994  Render_SetClipPlane    (near, far, halfW, halfH)
0x8002a25c  Render_SubmitScene
0x8002af98  Render_DrawVehicleHUD
0x8002b7bc  Render_DrawVehicle
0x8002b8d0  Render_DrawVehicleMuzzle
```

These functions occupy the seam between in-scope physics state and the
new renderer. Pass 2 will document the exact contract (what struct
fields are read from Vehicle, what GP0 primitives are produced) so the
replacement renderer can match the visible output without sharing
implementation.

## SHELL.DLL renderer additions

- `0x80104c88` -- `Shell_DrawSpritePair`: GP0 sprite emission, split
  into two prims when width > height. Renderer.
- ~80 other shell-UI functions in SHELL.DLL (level select grid, char
  select scroll, options screen, map screen) are renderer/UI: they
  build draw primitives, sample fonts, render text. Promoted bulk as
  out-of-scope -- the new shell UI will be re-written.
