# Vigilante 8: 2nd Offense guest roster UI audit

## Current state

The native guest-vehicle path is complete after selection: a wrapper choice
overrides the player type byte before COMMON resource-mask construction,
reserves native VRAM, builds independent body/transformation banks, allocates
stats and upgrade state, and dispatches the custom vehicle. Guest types begin
at 64 so they cannot alias the 18 retail vehicle identities.

What is missing is integration into `SHELL`'s player-select presentation. The
current desktop `Guest Vehicles` menu selects a package entry out of band; the
retail character-select screen continues to display whichever built-in slot
was highlighted.

## Native menu constraints

- `SHELL/SHELL.DLL` and `SHELL/CHARSEL1.TBL` /
  `SHELL/CHARSEL2.TBL` contain fixed retail roster navigation and presentation
  data. They are not append-only registries.
- The retail status/upgrade table has 18 records of 10 bytes. Guest vehicles
  already use independent host-owned upgrade records and must not extend this
  table in place.
- The match loader stores participant identities in signed bytes and builds
  retail resource masks from them. The existing guest hook handles type 64+
  before those masks are consumed.
- Character portraits, names, stat bars, faction ordering, cursor neighbors,
  and player-two exclusion state are separate concerns. Merely increasing a
  roster-count constant would index beyond several retail tables.

## Recommended integration

Implement a host-owned roster carousel layered over the native player-select
screen:

1. Preserve all 18 native slots and their exact navigation.
2. Add an explicit `Guest Vehicles` page reached from the end of each faction
   row (or with shoulder buttons). Do not renumber retail identities.
3. Draw guest portrait, vehicle/name text, faction, and four stat bars from
   package metadata in a host overlay. Extend the V8VR entry with stable
   portrait and faction metadata rather than embedding UI art in executable
   code.
4. On confirmation, retain the native shell slot for menu/state-machine
   continuity but record the selected guest type in `V82VehicleRegistry`.
   The existing pre-load override then supplies type 64+ to gameplay.
5. Maintain independent P1/P2 guest selections and enforce the native
   duplicate-selection rule unless `MIXED_CARS` is active.
6. Add keyboard/gamepad navigation, cancel, idle-demo safety, memory-card
   status isolation, and package-removal fallback to the built-in roster.

## Reusing first-game art

The first game's `SHELL/CHARSEL1.TBL` and `CHARSEL2.TBL` are available in the
loose tree and are suitable sources for original-driver portraits and panels.
Reuse must be by decoded texture/table entry, not by copying the whole first
game table over the sequel table: the layouts and sizes differ substantially.
The package should identify a source portrait by stable asset key and may
optionally provide independently authored replacement art.

## Risk and test matrix

The low-risk implementation leaves retail SHELL tables untouched. Patching
their counts and pointers directly is high risk because it couples guest count
to fixed cursor, portrait, unlock, status, and two-player arrays.

Required validation: 1P Arcade/Quest/Survival, 2P versus/co-op, locked and
unlocked retail rosters, zero/one/many guests, package removal, duplicate
selection with `MIXED_CARS` off/on, guest transformation previews, match load,
pause/quit, and return to player select.
