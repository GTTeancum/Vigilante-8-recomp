# Original quest integration — source findings

Implementation and validation, 2026-09-11. User authorized adapting the existing quest map
and filtering Quest character selection by actual campaign presence.

`audit_quest_sources.py` extracts original QUEST.BIN directly from the local
original PS1 disc, not the runnable roots: both currently have sequel data.
It decodes 12 original campaigns (48 missions) and 15 native campaigns
(120 missions). Original Y has no campaign; sequel Cultsmen, Boogie, Dusty
have null campaign pointers. Original Boogie does have a campaign.

Original layout: count, count/pointer pairs, 16-byte mission records.
Sequel layout: character count, three route pointers, character pointers;
routes have a count followed by 20-byte shared mission records, while each
character has 12-byte actor-count/actor-pointer/briefing-pointer records.
All actor descriptors are six bytes: type, flags, signed spawn, auxiliary.

Original mission +2/+4 fields are inclusive target object IDs, not weather
or audio. Confirmed from reference/generated/recompiled/main.cs
LAB_80022044 and FUN_800220d4; corresponding native V8:2 consumers are
8003223C and 800322D8. Both count matching scenery and evaluate the same
half-of-targets threshold. Native quest flags bit 7 selects destroy versus
protect; original main-loop XOR distinguishes the two original factions.
Original mission +1 selects a loading-card variant (LOAD 8010167C), not a
separate objective type. `src/assets/quest_loader.c` comments corrected.

Runtime implementation:

- General campaign records keyed by vehicle stable ID, with route/mission
  data and independently saved progress. Preserve native quest data and saves.
- Native character carousel eligibility comes from nonempty campaign
  records, preserving Arcade/AI roster availability.
- Native route-map and progress accessors now resolve custom types starting at
  64 through their own campaign records. They do not borrow a sequel campaign.
- Existing original arenas are native slots 8..17, corresponding to original
  arena IDs 0..9. Use existing map coordinates and previews.
- Source actor flags, spawn references, deferred boss records and scenery
  target ranges are translated into the native quest format. Resource loading
  reserves the authored opponents' models and specials, including the boss.

Audit artifact: `artifacts/original-quests-20260910/quest-source-audit.json`.
A test candidate is staged in V8_2_LOOSE. All 12 original characters and native
Sheila passed normal menu entry, the existing route map, authored spawn lookup,
and gameplay. Original Y is excluded; original Boogie is included. The native
15-campaign table remains intact, including its existing unlock requirements.

Integration defects found and repaired:

- Selector preview identity leaked into Quest acceptance. Acceptance now writes
  the selected stable runtime type before campaign lookup.
- Quest map background used a fixed resource index. It now uses the shared
  arena registry's background lookup.
- Original player spawn nodes can have arbitrary negative IDs. Original LOAD
  explicitly writes the player sentinel after construction; the adapter now
  does the same. Retaining a node ID such as -13 indexed camera pointers into
  timer bookkeeping. Native LOAD already assigns positive IDs to NPCs.
- Most reused original arenas have only XLSC selector 0; Airgrave and
  Canyonlands have 0..3. The converter inspects the installed EXP and retains
  the source card when available, otherwise choosing the first available card.
  Original briefing text and its objective color control are retained.
- Native quest-file teardown happens before gameplay. Imported progress must
  survive it, just like native fixed progress RAM. One reusable allocation of
  about 4 KiB holds custom progress and results text for the process lifetime.
- Native result row A is collection, B is scenery, C is combat. Original
  campaigns show scenery and combat as two rows, with no collection placeholder.
- Ending filenames need MIPS argument-home space below the filename buffer.
  The dispatch adapter reserves it before invoking the existing movie player.

Original ending paths are recovered from SHELL.DLL table 0x11C68, stride20, field+4. Media is extracted as full 2336-byte Mode2 sectors into distinct Movies/V8_*.str files; native movies remain unchanged. The campaign ending hook invokes the existing movie player.

RECOMPONE_QUEST_TEST_RESULT explicitly injects a win/loss/objective-failure result after600 gameplay frames for state-machine testing. This fixture is inactive in ordinary play and is not evidence of combat or objective logic correctness.

Reproduction:

```
python tools/recompone-v8-2/build_original_quests.py --output artifacts/original-quests-20260910/quests.json --install-endings V8_2_LOOSE
```

Stage that JSON as `V8_2_LOOSE/mods/v8_to_v82_guest_roster/quests.json`.
The converter reads the original disc and original SHELL ending table; no
hand-authored per-character campaigns or replacement maps are used.
`apply_native_quest_patches.py` installs the generated-code seams and is invoked
by the reference host build. `QuestContract` checks all 48 imported missions,
native payload preservation, pointer relocation, independent save slots,
player binding, progress lifetime and two-objective result arguments.

Proofs live in `artifacts/original-quests-20260910`. `roster-0` through
`roster-11` (Houston uses `houston-player-identity`) cover original menu entry;
`roster--1` covers native entry. Those roster runs used A78CDF99. Final UI and
lifetime repairs were tested as CE608401; final staging is 9AC53BE8. Individual proof JSON records identify
the exact executable used. Settings are restored byte-for-byte by the harness;
all input is process-local and captures come from the game itself.

Final progression checks on CE608401:

- `accepted-objective-fail`: Houston scenery failure displays A: No / B: Yes,
  saves mask 2 and returns through the native route flow.
- `accepted-reload-loss`: loads the actual earlier Chassey save, selects mission
  2 (Canyonlands), injects defeat and retains mask 3.
- `accepted-ending`: final Chassey mission succeeds, saves mask 31, plays all
  292 frames / 182 XA blocks of the original ending and returns to the menu.
- `accepted-native`: normal native Sheila Quest entry/briefing/gameplay passes.

`final-carousel` uses a process-only completed-native-progress fixture to
exercise the original sequel campaign tier checks. It visits all 27 eligible
characters (12 original, 15 sequel) in a complete cycle and enters a native
mission afterward. The Arcade unlock alone does not unlock sequel Quest tiers;
`accepted-carousel` records that deliberately rejected test setup. The final
build adds only this environment-gated fixture and removes a redundant save call
relative to CE608401. Its exact hash is recorded in `final-carousel/proof.json`.

Remaining acceptance limits: this is not a full combat playthrough of all 48
missions, all 15 native campaigns, or a co-op Quest certification.
Do not infer those from the explicit result-state fixtures. Original NPC easy
difficulty scaling has not been separately certified against the source game.


## September 11: Quest selector portrait repair

Quest passes A1=0 to the native selector to omit enemy setup. The imported
selector incorrectly treated that as an enemy-selection context, bypassing
portrait replacement and leaving Sheila's native portrait behind every car.
The host context now identifies Quest's zero-argument path as player one; the
native A1 remains zero, preserving the original Quest flow. No character-specific
fixes or asset substitutions were added.

Evidence: `artifacts/title-quest-selector-fix-20260911/proof.json`. One continuous
process navigated all twelve imported Quest cars; each settled screenshot was
visually inspected for the matching character, then Chassey's route and gameplay
were reached through the menu. QuestContract covers the zero-argument distinction
and unchanged non-Quest enemy context. Human Quest acceptance remains pending.

The same staged build fixes truncated title prompts by uppercasing the binding
label for the native uppercase title font. ControlsContract covers PRESS ENTER
and PRESS MENU; the native title screenshot visibly shows the complete PRESS MENU.
