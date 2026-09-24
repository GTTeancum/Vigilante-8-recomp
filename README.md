# Vigilante 8/2nd Offense PC Recompilation - Vigilante 8 Classic

An unofficial PC recompilation project for the PlayStation versions of
**Vigilante 8** and **Vigilante 8: 2nd Offense**. It's a two-in-one game!

## Gameplay Preview

[![Watch the Vigilante 8 Classic gameplay reel](https://img.youtube.com/vi/3hc6it7qC9M/hqdefault.jpg)](https://www.youtube.com/watch?v=3hc6it7qC9M)

**[Watch on YouTube](https://www.youtube.com/watch?v=3hc6it7qC9M)** — Work in progress — test footage.

## Screenshots

![Vigilante 8: 2nd Offense main menu](docs/screenshots/v82_main_menu.png)

<table>
  <tr>
    <td width="50%" align="center">
      <img src="docs/screenshots/v82_location_select.png" alt="Vigilante 8: 2nd Offense location select">
      <br>Location selection
    </td>
    <td width="50%" align="center">
      <img src="docs/screenshots/v82_route66_enhanced.png" alt="Enhanced Route 66 gameplay in Vigilante 8: 2nd Offense">
      <br>Route 66 with enhanced textures
    </td>
  </tr>
  <tr>
    <td width="50%" align="center">
      <img src="docs/screenshots/v82_houston_custom_color.png" alt="Houston's original Vigilante 8 Palomino with custom blue paint">
      <br>Original V8 Houston — custom blue paint
    </td>
    <td width="50%" align="center">
      <img src="docs/screenshots/v82_dallas_custom_color.png" alt="Dallas's Palomino XIII with custom purple paint">
      <br>Dallas — custom purple paint
    </td>
  </tr>
</table>

## Current Status

| Game | State | Current scope |
| --- | --- | --- |
| Vigilante 8: 2nd Offense | Windows x64 release | USA `SLUS-00868`, one- through four-player local play, GUI first-run disc import, enhanced presentation, and mods |
| Vigilante 8 | Playable development/reference build | All retail arenas, direct disc or complete loose-file play, one- and two-player testing, and long-form stability coverage |

The V8:2 candidate has passed fresh-directory first-run installation, repeat
launch, Route 66 gameplay, and invalid-disc rejection smoke tests. Broader
release regression work is still in progress.

## Highlights

- Original PS1 game logic recompiled for a modern 64-bit Windows host.
- Widescreen and selectable output resolution, with high-resolution 3D
  presentation and optional anti-aliasing.
- Enhanced shadows, lighting, fog, particles, texture filtering, mipmaps, and
  extended draw distance.
- High-resolution texture replacement and vectorized interface support.
- In-game video and control settings, including Modern, Trigger Drive,
  Classic, and Southpaw controller profiles.
- Movies, voices, sound effects, save data, and CD-audio playback.
- Standalone loose-file operation after the original disc has been imported.
- Mod loading from a distributable `mods` directory.
- A bundled V8:2 guest-roster mod containing all twelve original Vigilante 8
  vehicles, with player and AI support.
- Original Vigilante 8 arena compatibility work inside the V8:2 runtime.

## V8:2 Quick Start

1. Extract the entire release ZIP to a writable folder. Keep `mods` beside `Vigilante8PC.exe`.
2. Keep your USA **Vigilante 8: 2nd Offense** CUE and all referenced BIN files together.
3. Run **Vigilante8PC.exe**, click **Choose BIN / CUE...**, and select the CUE.
4. Wait for extraction and music preparation. The game starts automatically when setup completes.
5. On later launches, run the same executable; it uses `game_data` without asking for the disc again.

Requires Windows x64 and an OpenGL 4.5-capable driver. Runtime dependencies are bundled; no terminal or separate extractor is needed. Supported title ID: **SLUS-00868**. The importer checks this title ID and reads the selected disc's files and tracks, without requiring an exact disc/file hash.

[Full user instructions](notes/release/SETUP.md) cover controls, local multiplayer, troubleshooting and saves. The archive includes all five runtime mods, but no disc images or complete extracted base-game installation.

## Mods

Mods are isolated under `mods/<mod-name>/` and can supply manifests, textures,
vehicles, arenas, and other replacement content without overwriting imported
stock data. Keep the directory structure intact when moving an installation.

The release includes **all five runtime mods**: enhanced textures, Classic menus,
font replacement, the original-V8 guest roster (including its voice and quest
media), and **Super Dreamland 64**. Keep their folders intact. The enhanced texture
pack is part of the release configuration.

## Known Limitations

- This is the initial public release, not an exhaustive all-gameplay signoff.
- Automated setup tests use the application's own selection callback and framebuffer. Native Windows file-picker interaction and physical speaker output remain human checks.
- The supported first-run source is USA Second Offense, title ID `SLUS-00868`, with its complete CUE/BIN tracks.
- The optional original Vigilante 8 soundtrack is not on the Second Offense source; Second Offense music is used when it is unavailable.

The canonical backlog is [TO-DO.MD](TO-DO.MD).

## Project Structure

This repository contains the shared RecompOne runtime, game-specific
integration, supporting tools, mods, and verification material.

| Path | Purpose |
| --- | --- |
| `tools/recompone-reference/` | Shared PS1 runtime, host services, renderer, audio, input, and automation |
| `tools/recompone-v8/` | Original Vigilante 8 preparation and game-specific integration |
| `tools/recompone-v8-2/` | V8:2 preparation, host, patches, importer, and test tooling |
| `reference/` | Original Vigilante 8 reference-lane documentation and generated-work area |
| `reference-v8-2/` | V8:2 manifests, documentation, and generated-work area |
| `mods/` | Mod manifests and distributable mod content |
| `notes/` | Format research, defect investigations, and verification records |

Retail executables, extracted game assets, generated recompilation output,
local saves, logs, and release staging directories are intentionally excluded
from version control.

## Building

Development requires the .NET 10 SDK, Python 3, and legally obtained game
inputs. Recompilation preparation generates local source from the retail
binaries before the host can be built; those generated sources are not stored
in Git.

Start with the game-specific documentation:

- [Vigilante 8 reference lane](reference/README.md)
- [Vigilante 8: 2nd Offense reference lane](reference-v8-2/README.md)
- [V8:2 loose-file layout](reference-v8-2/LOOSE_FILES.md)

After the V8:2 reference sources have been prepared locally, the development
host builds with:

```powershell
dotnet build tools/recompone-v8-2/reference-host/Vigilante82PC.csproj -c Debug
```

## Testing and Reports

Automated runs cover startup, selectors, gameplay entry, weapons, AI, movies,
split-screen, result flow, teardown, and bounded multi-arena soaks. Visual
changes are reviewed from retained original-resolution and presentation
captures in addition to logs and deterministic acceptance data.

When reporting a defect, include the executable build identifier from
`v8_latest.log`, the affected game mode/map/vehicle, reproduction steps, and
the log itself when possible.

## Legal

The release omits disc images and the complete base-game installation. Supply
the base-game data from your own copy. Included runtime mods and their media
dependencies are described in the release manifest; ownership of game and mod
content remains with the respective rights holders.

Vigilante 8 and Vigilante 8: 2nd Offense are properties of their respective
rights holders. This fan project is not affiliated with or endorsed by
Luxoflux, Activision, or the rights holders.
