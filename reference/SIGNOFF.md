# Vigilante 8 PC Reference Sign-Off

This page contains only the checks that require human judgment. Automated
stability and gameplay gates are already complete; see `../TO-DO.MD` for the
full evidence log. Comparison against an externally instrumented PS1/emulator
is not part of this goal.

The images below are lossless conversions of the raw 320x240 reference
framebuffer captures, not photographs of the 1080p presentation window. They
therefore retain authentic PS1 color dithering. Enhanced-mode dithering removal,
perspective correction, and edge-only anti-aliasing are queued as the next
presentation phase and are not silently applied to this fidelity evidence.
Captured game frames are local-only evidence and remain intentionally excluded
by the repository's image/artifact ignore rules; this index itself is committed.

## Arena visual review

| Arena | Retained automated evidence | Visual |
| --- | --- | --- |
| Ski Resort | 120.36 s, 121 changing framebuffer probes, pass | ![Ski Resort](signoff/01_ski_resort.png) |
| Canyonlands | Match result, normal dismissal/relaunch, 112 changes, pass | ![Canyonlands](signoff/02_canyonlands.png) |
| Casino City | Match result, normal dismissal/relaunch, 103 changes, pass | ![Casino City](signoff/03_casino_city.png) |
| Valley Farms | 120.08 s, 136 changing framebuffer probes, pass | ![Valley Farms](signoff/04_valley_farms.png) |
| Hoover Dam | 120.54 s, 136 changing framebuffer probes, pass | ![Hoover Dam](signoff/05_hoover_dam.png) |
| Ghost Town | 120.22 s, 128 changing framebuffer probes, pass | ![Ghost Town](signoff/06_ghost_town.png) |
| Aircraft Graveyard | 120.47 s, 135 changing framebuffer probes, pass | ![Aircraft Graveyard](signoff/07_aircraft_graveyard.png) |
| Oil Fields | Match result, normal dismissal/relaunch, 95 changes, pass | ![Oil Fields](signoff/08_oil_fields.png) |

Sand Factory and Secret Base are locked by the preserved retail memory-card
state. The deterministic suite has fixtures for both but deliberately does not
rewrite the user's save or bypass the game's unlock system.

## Split-screen visual review

Both modes ran as clean, independent processes with simultaneous Player 1 and
Player 2 movement/weapon input, three observed Player 2 position changes,
distinct nonblack viewports, changing framebuffers, and normal in-game exit.

| Mode | Internal mode | Visual |
| --- | ---: | --- |
| Cooperative | 4 | ![Cooperative split screen](signoff/09_cooperative_split_screen.png) |
| Versus | 3 | ![Versus split screen](signoff/10_versus_split_screen.png) |

## Additional completed paths

Quest selected its vehicle and route, loaded Ski Resort, ran 900 active physics
ticks with movement and weapons, then exited through the original pause flow.

![Quest gameplay](signoff/11_quest_ski_resort.png)

The input-only victory test entered both cheats through the original passcode
editor, used ordinary pad input afterward, destroyed the sole opponent through
the retail damage path, and dismissed the genuine result screen normally.

![You Win result](signoff/12_you_win_result.png)

## Physical controller sign-off

Run `PS1 game/Vigilante8PC.exe` directly and check:

- Player 1 face buttons, D-pad/stick, triggers, Start, and weapon cycling match
  the on-screen Controller page and feel usable in gameplay.
- Dead zones hold a stationary car/cursor at rest but respond without an
  excessive center gap.
- Disconnecting and reconnecting the controller restores Player 1 input.
- Collision, weapon, and damage rumble have reasonable strength and stop
  promptly; no motor remains active after pause or exit.
- If a second controller is available, Player 2 receives its own controls and
  rumble rather than Player 1's motor commands.

Automation already proved SDL discovery of the attached `Xbox One Controller`,
a close/reopen rescan, default Player 2 bindings, and independent motor routing.
Only tactile feel and real unplug/replug behavior remain subjective.

## Deployed build

`BINCUE/Vigilante8PC.exe` and `PS1 game/Vigilante8PC.exe` are byte-identical:

```text
SHA256  1DFD56EFA33760E6E18072DFFCDB22775AF2520D9215FF7976971E302DA1D9C7
```

The first selects the neighboring CUE. The second selects the 69-file
standalone tree and never discovers or falls back to BIN/CUE data.
