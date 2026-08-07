# TO-DO 1 — "Vehicles will not reverse"

Reported shape: brakes stop the car but never reverse; the control stick does
reverse, while a face button or LT bound to the same command does not,
depending on the controller profile.

## The engine's reverse is not broken

Measured on the player vehicle's own fields rather than pixels or camera drift
(`include/structs.h`: `+0x20` inputLong, `+0x80` velX/Y/Z, `+0x10` 3x3
rotation). Signed forward speed is `dot(vel, third column of the rotation
matrix)`. Holding the digital Down bit from a coast, pad reporting `0x41`:

```
fwd  66,583  51,926  37,858  30,407  6,216  -1,755  -9,494  -21,286  -36,328
```

One zero crossing, monotonic decay, then steady negative acceleration. That is
textbook brake-then-reverse. **Reverse works on the plain digital Down bit.**

Reproduce with `tools/recompone-v8-2/run_reverse_probe.py` and
`RECOMPONE_V82_REVERSE_TRACE=1`.

## Gameplay input is digital only

Exhaustive search across every overlay: five references to the pad buffer
(`0x800B5298`, from `PadInitDirect`) and two to the 8-slot history ring
(`0x800B52E0`, filled by `func_80015104`). In gameplay the only consumer is
`func_80015540`, and its sole analog branch is gated `if (mode == 3)` and only
tests bytes for *exactly* `0xFF`, ORing in digital bits. No proportional value
reaches gameplay. The only proportional reader in the whole image is
`SHELL_SHELL.cs:11239` (`[padbuf+4+axis*2] - 0x80`, scaled, stored to
`obj+0x48`), which is the controller options page, not driving.

So proportional analog throttle is not something the game supports and cannot
be switched on. The engine ramps `inputLong` internally from a held digital
command.

## The pad ID, and why 0x23 must not ship

`func_800154A4` classifies the id byte at pad-buffer `+1`:

| id | mode | device |
|------|------|--------------------|
| 0x41 | 2 | digital pad (what we report) |
| 0x23 | 3 | Dual Analog |
| 0x53 | 4 | analog |
| 0x73 | 5 | DualShock |

`src/skipped/pad_input.c` records the same thing for V8: the analog axes are
sampled only when the type byte is `0x23`. DualShock's `0x73` is not a
substitute -- it classifies as mode 5, which nothing consumes.

Reporting `0x23` was tried and **degrades driving**. With axes centred the car
barely accelerates (coast fwd 66,583 -> 13,659) and reverse effectively stops
(39/84 samples clearly reversing -> 1/84). Mode 3 expects meaningful axes and
gets centre while the player holds a digital gas button. The change is reverted
and should not be reattempted without a matching axis feed; given the previous
section there is nothing proportional to gain by doing so.

## The binding layer checks out

The reporter's `settings.json` is `InputBindingsVersion 2`, profile
`Trigger Drive`, `Down = [100, 12, 105]` (LT, D-pad down, left stick down).
`InputBindingResolver.ResolvePad` returns the configured layout whenever
gameplay is active, and `GpuHle.GameplayActive` is only cleared on genuine
shell transitions (`LibCd.cs:206`, `V82Compat.cs:4896`), not mid-match. The
native-menu pulse that would swap in the stock layout fires only on the exact
strings PAUSED / QUEST OBJECTIVES / ARE YOU SURE?. `Pressed` handles LT through
`GameControllerGetAxis(Triggerleft)` correctly.

Note that `InputProfiles.ValidateTriggerDrive` and
`ValidateModernBrakeReverse` are dead code -- defined, never called.

## What was changed

Analog bindings (triggers and sticks) now latch with hysteresis: they press at
the full threshold and release at three quarters of it, tracked per pad and per
binding. A bare threshold compare makes an input held near the boundary chatter
the command on and off frame to frame, and reverse is precisely the command
that cannot tolerate that -- it only engages after an unbroken hold through the
stop, so one dropped frame restarts the wait. Triggers sit closest to this
because they engage at 8000 of 32767 (24%) against the sticks' 16000 (49%).
`Apply` now evaluates every binding rather than returning on the first hit, so
the latch state of the rest stays current.

## Root cause: the effective input profile depended on the launch directory

`settings.json` and `interface.ini` were bare relative paths, so they resolved
against whatever directory the process was started from. The working directory
is never normalised (`Program.cs` only records it). A working tree plus a
staged install holds several copies, and they disagree:

```
./settings.json                                  Modern         Down=[2,12,105]
./V8_2_LOOSE/settings.json                       Trigger Drive  Down=[100,12,105]
./reference-v8-2/.../win-x64/settings.json       Modern         Down=[2,12,105]
./PS1 game/settings.json                         Classic        Down=[12,105]
```

Under **Classic** the reported symptom is exact and complete:

| input | Classic binding | result |
|-------|-----------------|--------|
| left stick down (105) | Down | brake, then reverse |
| d-pad down (12) | Down | brake, then reverse |
| face X (2) | Square = "Brake" | stops, never reverses |
| LT (100) | L2 = fire weapon | nothing |

So a player who configures Trigger Drive in one location and launches from
another gets Classic or Modern bindings instead, and brake/reverse survives
only on the stick and D-pad. That is "the control stick works as reverse, face
buttons or LT do not, depending on controller config".

`settings.json` now resolves next to the executable, deterministically,
migrating a config found beside the launch directory the first time so nothing
is lost. Startup logs the resolved path, the active profile and the Down
bindings, which makes this class of problem self-evident:

```
[Config] settings=...\win-x64\settings.json profile=Trigger Drive down=[100,12,105]
```

`interface.ini` is deliberately left alone: the soak determinism tooling
restores it between runs and moving it would disturb that.

## Verification

With the Trigger Drive config loaded, driven through the real binding chain
(profile resolution, binding arrays, hysteresis latch) using a synthetic pad:

| input | native bit | final signed forward speed | |
|-------|-----------|---------------------------|---|
| LT | Down | -32,549 | reverses |
| left stick down | Down | -23,764 | reverses |
| A (bound to L2) | none | - | correctly no Down |

## Not resolved

Ordinary scripted pulses write `Controller.State` directly and bypass profiles,
`ResolvePad` and `Pressed`, so the binding layer had no headless coverage at
all. Input scripts now also accept physical inputs -- `PHYS:LT`, `PHYS:A`,
`PHYS:LSDOWN`, `PHYS:DPADDOWN` and so on -- which are routed through the real
resolution path against a synthetic controller. That is what made the profile
difference visible and what the verification table above is measured with.

One device-side possibility remains untestable from here: a controller whose
SDL mapping does not expose LT as `Triggerleft` (some older drivers report both
triggers on a single shared axis) would still fail while stick and D-pad work.
The startup `[Config]` line plus a `PHYS:` probe run distinguishes that from a
profile problem in one go.
