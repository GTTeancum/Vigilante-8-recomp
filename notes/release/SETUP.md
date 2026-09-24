# Vigilante 8 Classic — getting started

## What you need

- A 64-bit Windows PC with an OpenGL 4.5-capable graphics driver.
- Your USA PlayStation **Vigilante 8: 2nd Offense** disc image, title ID **SLUS-00868**.
- The **CUE and every BIN file it references**, kept together. The original Vigilante 8 disc, a Dreamcast disc, an ISO alone, or a lone BIN without its matching CUE will not work for setup.
- A writable folder with room for the release and extracted game data. Allow **2 GB free** during setup, in addition to your source disc files.

.NET and the native runtime libraries are bundled. No terminal, Python, extractor or separate runtime installation is needed.

## Install and play

1. Right-click the release ZIP and choose **Extract All**. Open the extracted `Vigilante8Classic` folder. Do not run the game from inside the ZIP.
2. Keep the entire `mods` folder beside **Vigilante8PC.exe**. You can keep your disc image elsewhere; its CUE and BIN files must stay together.
3. Double-click **Vigilante8PC.exe**.
4. In the setup window, click **Choose BIN / CUE...**. Select your **Second Offense CUE**. Selecting one of its BIN files also works when exactly one adjacent CUE references it.
5. Wait while the progress bar advances through **Extracting game files...** and **Preparing music...**. Setup creates `game_data` beside the executable. Music conversion can take longer than file extraction.
6. After **Installation complete**, the game opens automatically in the same launch. Press **Start** on a gamepad or **Enter** on the keyboard, then follow the on-screen prompts.

On later launches, double-click the same executable. Setup will not ask again, and the source BIN/CUE files are no longer needed for normal play. Keep `game_data` and `mods` with the executable.

## Playing and controls

Choose **1 PLAYER**, **MULTIPLAYER**, or **OPTIONS** from the main menu. Use **OPTIONS** to select the display resolution, soundtrack and control preset. Follow the prompts for your current input device; they change with the selected control layout.

For local multiplayer, join with gamepad **A** or its equivalent. Player one confirms with **Start** or **Enter** once at least two players have joined. Up to four players are supported, including a keyboard plus gamepads. Multiplayer uses at most six combatants total.

All five runtime mods are supplied: enhanced textures, Classic menus, the font replacement, the original Vigilante 8 crew, and Super Dreamland 64. Keep the enhanced texture pack active. Guest voices and quest-ending media are included with the roster mod.

The saved soundtrack preference defaults to **Vigilante 8**. This release does not bundle its soundtrack; a Second Offense-only installation plays Second Offense music as a fallback. You can explicitly choose **Second Offense** in the soundtrack option.

## If setup does not finish

- **Wrong title:** select the USA Second Offense disc, SLUS-00868. Eligibility uses the title ID, not a disc/file hash match.
- **Missing BIN or audio track:** restore every file named in the CUE and keep the original filenames. Do not select just the data track from an incomplete download or copy.
- **BIN is ambiguous:** select the matching CUE directly.
- **Interrupted or canceled extraction:** launch again and select the source again. Setup retries the unfinished `game_data.partial` import; it does not treat it as complete.
- **Incomplete `game_data` folder:** preserve a backup of that incomplete folder outside the installation, then retry setup into a fresh release folder. Do not merge arbitrary game-data folders.
- **Cannot create files:** extract to a folder you can write to, rather than running inside the ZIP or a protected system folder.
- **No setup/game window or graphics error:** check that your graphics driver supports OpenGL 4.5. When reporting a problem, include `v8_latest.log` and describe the last screen you saw.

Canceling the Windows file picker returns to setup without starting an import. **Exit** closes setup; **Cancel installation** stops an active import.

## Saves and updating

Settings and saves are created beside the executable (`settings.json`, `carda.sav`, `cardb.sav`, and any quest-progress files). Back up these files and your installation before replacing a release. This ZIP contains no personal settings, saves, source disc images or complete base-game installation.

This is the initial v1.0 release. The setup/game flow has automated smoke coverage; native file-picker operation and physical speaker output still require a human check.
