# Ambiguous

Functions whose subsystem assignment is unclear.

## Pass 1

- **Many `auto/` stubs in `src/gameplay/`** may actually belong to
  `src/assets/` because they participate in shell-screen flows that mix
  asset loading with UI. The classifier currently routes by address
  range (0x80011000-0x80015000 -> gameplay) but several of those
  addresses are PSY-Q-style file helpers we haven't relabeled yet.

- **GPU otag / primitive builders** (`FUN_8001d994`, `FUN_8002a25c`,
  `FUN_8002b7bc`, ...) are called from the main loop but produce
  display lists, not physics state. They sit on the renderer seam --
  the new renderer consumes their outputs, but the *call sites* are
  in the 1:1 zone. Tagged ambiguous: the call site stays 1:1, the
  callee is renderer.

- **Sound emitter calls** (`FUN_80043df8("Sounds\\Victory.xa", ...)`)
  are gameplay decisions (when to play) but the implementation is the
  audio engine. Treat as event-emit per the seam.

## Resolution policy

For each ambiguous case, pass 2 will:
1. Cite the seam in `PROJECT_SCOPE.md > The Seam Contract`.
2. Place the *call-site* code in `src/gameplay/` or `src/physics/` as
   appropriate.
3. Place the *implementation* under `src/skipped/` (declarations
   only) with a doc-comment showing the seam contract.
