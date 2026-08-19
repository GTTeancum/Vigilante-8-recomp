# Vigilante 8 N64 reference harness

This harness runs the original N64 ROM as a deterministic source reference.
It is deliberately separate from the PS1 recompilation and conversion tools.

## Safety and desktop isolation

- The emulator is the portable RMG distribution. Nothing is installed and no
  registry integration is performed.
- The worker runs on a private Win32 desktop that is never switched into view.
- Input is supplied by a dedicated Mupen64Plus plugin from a frame-indexed CSV.
  It does not send global keyboard or mouse input.
- Captures come directly from the active video plugin through `ReadScreen2`.

## Evidence

The GFX proxy records one frame identity across:

- the rendered framebuffer;
- raw RSP DMEM and IMEM;
- the active RDRAM image when requested;
- DPC start/end/current/status;
- VI origin and width.

This is intended to answer narrow source-contract questions such as Dreamland's
road geometry, submission order, and depth/render behavior before considering a
full N64 recompilation.

## Build and run

```powershell
.\tools\n64_reference\build.ps1
.\tools\n64_reference\run_trace.ps1 -StopFrame 900 -CaptureEvery 60
```

`build.ps1` writes `bin/build_manifest.json` with the compiler command plus
source and binary SHA-256 values. `run_trace.ps1` copies those identities into
the run manifest alongside the ROM, portable runtime, real GFX/RSP plugins,
and scripted-input hashes. The default `-AudioMode null` uses a no-thread audio
sink so host playback cannot perturb the reference. `-AudioMode trace` wraps
the stock RMG audio plugin and records AI DMA callbacks for focused diagnosis.
The RSP proxy always forwards CXD4 unchanged and records graphics/audio task
headers by emulated frame.

The focused Super Dreamland spawn-road capture is:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  .\tools\n64_reference\run_trace.ps1 `
  -InputScript input_scripts\dreamland_gameplay_probe.csv `
  -StopFrame 5900 -CaptureEvery 0 `
  -CaptureFrames '5100,5160,5220,5280,5340,5400,5460,5520,5580,5640,5700,5760,5820,5880' `
  -DumpRdram -DumpRdpCommands -Profile angrylion-cxd4 `
  -Output artifacts\n64_reference\dreamland-spawn-road-reference
```

Input rows use:

```text
first_frame,last_frame,buttons,analog_x,analog_y
```

Buttons are joined with `+`: `A`, `B`, `START`, `Z`, `L`, `R`, `DU`, `DD`,
`DL`, `DR`, `CU`, `CD`, `CL`, and `CR`.

The buttons field also accepts `PASSCODE:xxxxxxxxxxxxxx`. At the row's first
frame, the input plugin expands that string into debounced native D-pad
movements using the game's `- A..Z 0..9 @` character order, moves right between
the 14 slots, and presses A after the final slot. This drives the retail
passcode screen; it does not patch emulated memory.

The portable archive is RMG v0.9.0 from the project's official GitHub release.
Its SHA-256 is:

`F81640A0AE7474C0915F263D1ACAC20CF816AC8666F897CBF3710CD485A9027D`

The extracted runtime and archive under `vendor/` are local tooling inputs and
are ignored by Git.

## Static surface-contract proof

The dynamic trace is paired with a source-level gate that follows Dreamland's
road from the arena chunks into the recovered PS1 loader and the Enhanced
renderer:

```powershell
python .\tools\n64_reference\verify_dreamland_surface_contract.py
```

The resulting `artifacts/n64_reference/dreamland_surface_contract.json`
asserts that all 35 `RSEG` records and both `XRTP` descriptors remain present,
that the primary road is a textured route whose edge vertices are sampled from
`Terrain_HeightAt`, that its N64 runtime triangles are opaque Z-writing despite
the platform-specific source `0x0100` flag, and that only
source-identified water or actual glass receives modern transparent depth
testing. It also rechecks the retained road-stability and water-occlusion
positive/negative-control traces.

## Spawn-road combined proof

`verify_dreamland_spawn_road_reference.py` requires every requested frame to
contain one framebuffer, matching RSP DMEM/IMEM/RDRAM dumps, one RSP identity
record, and valid indexed RDP byte spans. `--repeat-capture` additionally
requires byte-identical framebuffer, DMEM, IMEM, VI/display-list identity, and
ordered RDP payload hashes across two hidden runs. At the audited route frame,
the active display list, every referenced route vertex, and the XRTP-0 texture
payload must also match byte-for-byte.

Whole-RDRAM hashes are retained as diagnostics rather than a render-
determinism gate. Paired null-audio runs prove that every graphics RSP task is
identical and that residual task variance is confined to the `data_size` field
of type-2 audio command lists. The combined proof requires framebuffer,
graphics-task, ordered RDP, and complete route-relevant RDRAM identity while
reporting audio scheduler state separately.

## End-to-end build provenance

`verify_dreamland_build_reproducibility.py` audits the complete original-V8
CUE-to-RecompOne build boundary. It closes and hashes every CUE track,
fingerprints the generator/runtime and generated C# trees, executes both
post-patch checks, validates the deterministic prehook, and requires the first
published executable to predate the independently regenerated source set and
the repeated executable to follow it. Both executables and the deployment must
be byte-identical.

```powershell
python .\tools\n64_reference\verify_dreamland_build_reproducibility.py `
  --cue "BINCUE\Vigilante 8 (USA).cue" `
  --config reference\generated\v8.recompone.json `
  --generated-dir reference\generated\recompiled `
  --first-exe artifacts\n64_reference\first\Vigilante8PC.exe `
  --second-exe artifacts\n64_reference\second\Vigilante8PC.exe `
  --deployed-exe "PS1 game\Vigilante8PC.exe" `
  --output artifacts\n64_reference\dreamland-build-reproducibility.json
```
