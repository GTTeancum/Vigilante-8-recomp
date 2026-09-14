# Original V8 Houston and Y source port — 2026-09-10

The staged V8:2 build uses the original PS1 Houston/Y weapon behavior and Y's
original flying physics. Houston no longer selects the V8:2 tow-truck overlay.
The original source assets retain their model indices and articulation. Y has
no synthetic wheel/contact anchors or invented water-height clamp.

## Source authority and the recovered error

Original generated instructions in `reference/generated/recompiled/main.cs`
are the authority where cleaned C disagrees:

| Behavior | Original entry |
|---|---|
| Houston aim, fire, equip, AI query | `80035CF8` |
| Y aim, fire, equip, AI query | `8003607C` |
| Shared beam propagation and hit | `800359C0` |
| Mounted effect retirement | `8003E80C` |
| Hit effect / eight sparks | `80040234` / `8004007C` |
| Y construction, no wheel construction, mass `0x5000` | `80030F34`, event 7 |
| Y movement and damping | `80030C08` |
| Position / GTE rotation integration | `80017324` / `800439B8` |

`src/gameplay/vehicle_init_joints.c` had incorrectly reconstructed `800359C0`
as a projectile with a nine-update lifetime and a generic impact. The original
instead propagates eight more animated beam segments, first after two updates,
then after three updates per generation. Animation-end event 5 retires each
segment. Collision does not immediately free it. The cleaned C is corrected.
The first candidate's unit tests repeated that mistaken reconstruction; those
results are superseded by the independent generated-code comparisons below.
That rejected candidate also crashed at destruction and is not staged.

The port preserves Houston's 50 damage, Y's 100 damage, the original model
indices, signed 12-bit aiming, Houston's pitch limits, sound choices, firing
delays and beam generation count. The collision effect uses original COMMON
bank 14 roots `0x15/0x16` and Main.SND `0x41`; equip uses Main.SND `0x2B`.
Source vehicle sound tables remain unchanged at their original first three
indices; the two common samples are appended. The impact bank metadata follows
the asset bank, so freeing the owner does not invalidate a remaining beam.

## Integration boundaries

`V8OriginalSpecials.cs` adapts object offsets, event numbers and engine services
to V8:2. It does not borrow a retail special DLL. The special constructor resolves
an explicitly tagged original callback after the ordinary native lookup. Stock
vehicles keep the unmodified native callback. The package converter no longer
reorders Houston's roots or inserts children to satisfy the tow-hook model.

Y's original physics replaces the native ground/transform physics at its
dispatch point; it is no longer added before a second native movement step.
The native common epilogue retains weapon timers and follow-position maintenance;
the source snap branch is selected for flying vehicles. Rotation executes the
original generated GTE instruction sequence. Terrain/obstacle probing and PSYQ
length/normalization use corresponding engine services. Native input, general
vehicle lifecycle and rendering remain V8:2 integrations, not a complete original
V8 engine transplant. Y's previously sourced selector audio is retained.

## Validation

- `SpecialResourceContract`: **3,293 checks pass**. Independent oracles extracted
  from original generated instructions compare beam update/propagation/retirement,
  collision return/effects, and 128 varied Y movement states. The flight comparison
  covers the matrix, positions and velocity values. Engine services are controlled
  fixtures in these comparisons; full integration is tested separately below.
- Conversion: **14 tests pass**. Full package audit passes for 13 vehicles,
  38 owned banks, 780 texture records and exact source environment-role mapping.
- Blender save/reopen/export is byte-exact to the new converted package.
  An accumulated multi-vehicle Blender process crashed; resuming the remaining
  vehicles in separate background processes succeeded, followed by finalization
  and the full package audit. Runtime metadata survives the authored scene now.
- Corrected C passes Clang syntax checking with two pre-existing warnings outside
  the changed function.

Native hidden, process-local runs in Utah, with six captures each:

| Run | Fires | New segments | Hits | Segment animation retirements | Exit / defeat / shell |
|---|---:|---:|---:|---:|---|
| Houston, held firing fixture | 5 | 40 | 15 | 45 | Pass |
| Y, held firing fixture | 5 | 40 | 17 | 45 | Pass |
| Y, unrestrained driving fixture | 6 | 48 | 3 | 54 | Pass |
| Stock V8:2 tow truck | Native | No source callbacks | Native | Native | Pass |

All four runs have no fatal error or edge pool drops. Native captures were
visually inspected for the mounted gun, firing/hit effects, Y movement and stock
tow behavior. The whole-map gate intentionally rejects multiple captures and
some held routes; its overall return code is **not** represented as a pass.
The separate `source_special_acceptance.json` files evaluate only these functional
checks. FPS enforcement was disabled. This is not an all-level regression claim.

Evidence: `artifacts/houston-history-20260909/{houston-final,y-final,y-flight-final,stock-tow-final}`.
Reproduce with `tools/recompone-v8-2/run_original_special_acceptance.py`, e.g.:

```powershell
python tools/recompone-v8-2/run_original_special_acceptance.py --output artifacts/source-special-repeat --map-slots 16 --player-type 71 --gameplay-frames 360 --timeout 240 --unthrottled --hold-for-capture --capture-gameplay --capture-gameplay-poll 350
```

Use player 76 for Y, omit `--hold-for-capture` for movement, or use player 3 for
the stock tow truck. The harness never generates host input.

## Staged files

`V8_2_LOOSE/Vigilante82PC.exe` SHA256:
`A6A307099366ED44E4EC05B3C3926624EB0205E75C84EC40A9136BA1C241984E`

`mods/v8_to_v82_guest_roster/CUSTOM.EXP`:
`179051A6F215EBFE6C436EC032B566E2BB916C243DE253F7B325E106A2D951C5`

`mods/v8_to_v82_guest_roster/VEHICLES.V8R`:
`B64295BD322503FDE5FC0FCFD2C92716C5AE537D9F8A4E9AC25CCD7C93696846`

No Dreamland changes, commit or push were made for this source-port request.
