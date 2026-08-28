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

## Required integration

The rejected desktop/ImGui roster prototype is not a product path. Guest
vehicles must extend `SHELL`'s existing player-select state machine and use its
native layout, text, arrows, three-dimensional preview, stat rows, animation,
sound, one-player flow, and two-player flow.

1. Preserve all 18 native slots and their exact navigation.
2. Add an explicit native legacy page reached from the existing selector.
   Do not renumber or replace retail identities.
3. Feed guest names, native stats records, preview banks, and authentic
   first-game driver-banner assets through the same native rendering calls
   used by the retail selector. Host-rendered substitute panels are forbidden.
4. Confirm a guest as its append-only type 64+ identity. The match loader and
   independent gameplay constructor already accept that identity.
5. Maintain independent P1/P2 guest selections and the native duplicate rule.
6. V8 legacy entries expose only their one canonical standard body. The
   selector must hide/disable V8:2's Stock/Hot Rod control for those entries;
   no retail Hot Rod bank may be borrowed or implied.
7. Keep the wrapper `Guest Vehicles` item solely for installing or changing a
   test package.

## Reusing first-game art

The first game's `SHELL/CHARSEL1.TBL` and `CHARSEL2.TBL` are available in the
loose tree and contain the authentic left-side driver banners. Reuse must be
by the matching decoded entry, not by copying an entire table over the sequel
table: the layouts and sizes differ substantially.

The first three V8 menu categories are compatible with V8:2. Their authored
armor, speed, and handling values are reused exactly with the format's exact
10x scale conversion (V8 0..20 to V8:2 0..200). V8's selector presents only
three categories and every retail fourth byte is zero. V8:2's mandatory
fourth/special row is therefore a documented sequel-only compatibility value,
not claimed as original V8 data.

## Risk and test matrix

The low-risk implementation leaves retail SHELL tables untouched. Patching
their counts and pointers directly is high risk because it couples guest count
to fixed cursor, portrait, unlock, status, and two-player arrays.

Required validation: 1P Arcade/Quest/Survival, 2P versus/co-op, locked and
unlocked retail rosters, zero/one/many guests, package removal, duplicate
selection with `MIXED_CARS` off/on, guest transformation previews, match load,
pause/quit, and return to player select.

## Conversion follow-ups

Review status: the Y conversion, flying-controller capability, non-transformable
gates, selector/AI integration, and text-only regression evidence are staged in
`V8_2_LOOSE`. The items remain open until the user performs the required
visual/gameplay review; they are not being marked complete by automation.

- [ ] Convert OG V8's Y the Alien and his flying saucer as a distinct legacy
  guest entry, even though V8:2 has its own native Y vehicle. Route the saucer
  through the same V8-to-V8:2 model, packet, material, selector, registry, and
  gameplay conversion infrastructure as every other imported vehicle.
- [ ] Represent the saucer as a generic non-transformable vehicle capability.
  Non-transformable vehicles must be ineligible to collect transformation
  powerups. The check must consume capability metadata; it must not branch on
  Y's display name, stable ID, numeric type, model, or source game.
- [ ] Validate ordinary weapon/powerup collection, flight behavior, selector
  presentation, reflection/material output, AI use, and 1P/2P match loading,
  plus explicit rejection of all transformation pickups without consuming or
  activating them.
