# Loose-file reference run

The same reference executable is deployed to two locations. Neither copy
requires a command script.

- `BINCUE/Vigilante8PC.exe` finds the neighboring Vigilante 8 CUE and
  runs directly from the original disc tracks.
- `PS1 game/Vigilante8PC.exe` finds `../BINCUE/Vigilante 8 (USA).cue`
  automatically and treats `PS1 game` itself as the loose-file override root.

The two executable files must be byte-identical; their location selects the
setup. The loose tree mirrors the ISO paths exactly. Examples are
`PS1 game/COMMON.EXP`, `PS1 game/HUD/ARMS.EXP`,
`PS1 game/SHELL/VEHICLES.EXP`, and
`PS1 game/TERRAIN/OILFIELD.EXP`. A matching loose file supplies its current
length and bytes. Files not present in the loose tree continue to come from the
disc, and edited loose files are picked up on the next launch without rebuilding
a BIN.

The CUE remains required by this reference implementation. It supplies stable
original LBAs for the recompiled loader plus raw sectors for XA/STR streaming
and Red Book CD audio. Loose data files override ordinary 2048-byte ISO reads;
raw audiovisual sectors remain disc-backed. Replacing MIPS instructions inside
`SLUS_005.10` or a `.DLL` does not create newly recompiled host code, although
their data portions can still be overridden.

Optional command-line forms are:

```text
Vigilante8PC.exe [disc.cue]
Vigilante8PC.exe [disc.cue] --loose <directory>
Vigilante8PC.exe [disc.cue] --no-loose
Vigilante8PC.exe --probe-source
```

The executable defaults Vigilante 8's own Music and Sound Effects sliders to
zero, matching the former launcher behavior without altering Windows volume.
