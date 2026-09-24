# Release manifest - program plus ALL runtime mods

The distribution includes `Vigilante8PC.exe` and all five runtime mods. The user
supplies the Second Offense PS1 BIN/CUE; the base-game installation is not shipped.

```text
Vigilante8Classic/
|-- Vigilante8PC.exe                  # GUI setup built into the executable
|-- SETUP.md                          # notes/release/SETUP.md
|-- README.md                         
|-- THIRD-PARTY-NOTICES.txt
|-- dependencies.json, license-sources.json
|-- licenses/                        # 17 component license/notice files
|-- mods/
|   |-- enhanced_textures_2x/          # REQUIRED; included and active
|   |   |-- manifest.json
|   |   |-- images/**/*.dds
|   |   |-- loading_cards/*.ppm
|   |   `-- files/{SHELL/VEHICLES.EXP,SHARED/COMMON.EXP} # authored light masks
|   |-- v8_classic_menu/
|   |   |-- manifest.json, README.md
|   |   |-- images/**/*.dds
|   |   `-- files/SHELL/
|   |-- ttf_game_font/
|   |   |-- mod.json
|   |   `-- files/SHARED/GAME.FNT
|   |-- v8_to_v82_guest_roster/
|   |   |-- CUSTOM.EXP, VEHICLES.V8R, quests.json
|   |   |-- SHELL/SELECTOR_*.PPM
|   |   `-- files/
|   |       |-- SHELL/{SOUNDS.SND,V8VOICES.SND}
|   |       |-- SHARED/V8VOICE/{D00..D12,V00..V12}.XA
|   |       `-- MOVIES/V8_*.STR        # 12 guest quest endings
|   `-- v82_n64_super_dreamland/
|       |-- ARENAS.V8R
|       |-- files/LEVELS/N64/{DREAMLND.DLL,DREAMLND.EXP}
|       |-- files/SHELL/LEVELSEL.TBL
|       `-- loading_cards/*.ppm
`-- release-manifest.json
```

All runtime assets are included. No proof captures, font production/source
intermediates, disc images, original loose base-game directory trees, personal
settings, saves, test logs or obsolete root vehicle registries are shipped.
The selected guest media and authored vehicle banks above are dependencies of
mods, not a complete base-game installation. They were previously stored outside
the mod folders and were missed by the initial packaging inventory.

On first launch the GUI accepts Second Offense USA title ID SLUS-00868 without
hash eligibility checks, extracts files and music with a progress bar, then
starts the game. Subsequent launches use the generated `game_data` directory.
There is no separately shipped `setup` program or terminal step.

Original Vigilante 8 remains the saved default soundtrack preference. Its music
is optional and not present on the Second Offense source; a Second Offense-only
installation uses Second Offense music. The guest mod includes its required
selector/result voices and quest endings.

## Prepared candidate — 2026-09-24

Local archive: `artifacts/release-20260924/package-full/Vigilante8Classic-2026-09-24-win-x64-rc.zip`.

- 302,735,162 bytes compressed; 4,438 files including the integrity manifest.
- 596,174,127 payload bytes before the manifest; all five mods included.
- ZIP SHA-256: `22fe86e7fca06022f5b95e1f343066e68e82f0367114f8343c9cf88cddd8b315`.
- Executable SHA-256: `2a63834d1851c6d99becbc8790fce7f63d9e4cf84962269131dd86685cc1c34e`.
- Built locally on main at `5e3a418`, including the uncommitted GUI setup work. This is not a reproducible build from that commit alone.

The exact inventory is the archive's `release-manifest.json`, also available under
`artifacts/release-20260924/package-full/Vigilante8Classic/`. Integrity hashes
are packaging metadata, not disc acceptance checks. The packager validates every
manifest-referenced mod asset, including nested DDS directories.

[Setup instructions](release/SETUP.md) and component notices/licenses are included
in the ZIP. No blanket project license is asserted by component attribution files.
[Release smoke evidence](release_smoke_20260924.md) records passes and limits.

No archive has been published. Both executable aliases are staged in the protected
`V8_2_LOOSE` root. The earlier `artifacts/release-20260924/package/` candidate is
INVALID (missing nested DDS assets); only `package-full` is the prepared release.
