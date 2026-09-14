# Color customization and result voice repair — September 13

Scope: TO-DO #7 / V82-OPEN-027 and #9 / V82-OPEN-029. The user closed the
broad renderer item #1. Utah remains separately open and the soak stays pending.

## Result voice source evidence

The deployed user log selects `guest.v8.houston_3`, victory `V8VOICE/V07.XA`,
and normalizes its filter to channel zero. Selection of the output filename
is correct; the source payload inside it was wrong.

Original PS1 `analysis/SLUS_005.10/decomp/80013cac.c` passes the selected vehicle
index `DAT_80065674` unchanged to `FUN_80043df8` for the victory/defeat bank.
`analysis/SLUS_005.10/decomp/80043df8.c` assigns its second argument unchanged
to the channel byte passed to CdSetfilter. No increment or cyclic rotation
occurs. The selected vehicle index also selects the vehicle bank and quest row
in the same original function. Correct source channels are therefore 0..12.

The converter and its verifier previously assumed channels 1..12,0. That
assumption shifted every ordinary driver's voice and assigned Y the first
channel. Rebuilt 26 files with source channels 0..12. Every coding byte and
ADPCM payload byte is unchanged from the corresponding original bank sector;
only channel/EOF headers and native stream cadence are adapted. Verification:
`artifacts/custom-color-result-voice-20260913/voice-content-proof.json`.

Runtime replay and customization reproduction are in progress. Do not treat
this content audit as proof of runtime playback or audible acceptance.

## Customization source repair and ownership

The crashing staged build requested 3221225569 bytes from native color-copy
return address 0x8001EFF4 after entering Houston's editor (0x801074AC). The
compiler emitted only N group offsets; native assets contain N offsets followed
by the group block's byte size. The color copier uses that terminal size word.
Both original and sequel COMMON.EXP banks confirm size = collision-table offset
minus group-table offset. The shared compiler now emits that word for both
formats. The accepted staged roster and a fresh build from original sources
were semantically identical; recompilation preserves all 13 vehicles exactly.

After this correction the editor no longer crashed, but its native proxy row
also accumulated colors across imported characters. Three durable seams now
route editor record access to an owned selector record. The existing native
color routine receives the imported preview bank at commit/restore, saved
colors are keyed by registered stable identity, and gameplay bank construction
uses the same native color routine. Cancel retains the prior color; accept
persists it. Selector record and copied group-block allocations are released
with their owning preview. Stock vehicle paths remain native.

The 12 Quest-eligible imported characters passed cancel/accept with independent
values in one process. Y is intentionally not in that Quest carousel; Arcade
coverage is separate. Original/native gameplay and voice replay remain in
progress until the final reports are written.

Conversion tests: 15 passed. Broader native-format tests: 35 passed, 2 skipped,
2 pre-existing failures in transformation collision-only authoring validation.
Those same two failures were reproduced with the pre-fix compiler in an isolated
Python interpreter; see native-format-baseline.log. They occur during source
validation, before compilation, and are outside these two reported defects.

## Final verification and staging

Final executable SHA256: `87127ec9d0b0f9df38c7cf5928af14356a1b8b9656bded9014cb637fec2dd192`.
The executable, regenerated CUSTOM.EXP/VEHICLES.V8R and all 26 voice files are
staged in V8_2_LOOSE and match the verified artifacts byte-for-byte.

- All 13 original vehicles passed color entry, change, cancel and accept in
  one continuous Arcade selector run, with native vehicles before and after.
  Each guest retained zero on cancel and independently saved four on accept.
  The 12 eligible Quest characters passed the equivalent test separately.
- A new process loaded Houston's saved color into gameplay and completed his
  first quest through the result fixture. His original victory line streamed
  all 39 sectors to file end: 157248 stereo PCM frames at 37800 Hz, 4.16 seconds.
- Native Sheila's result path remained SHARED/SHEILA.XA, played all 16 accepted
  sectors to file end, and did not use the guest override.
- All 26 imported outcome files match the original channel's coding and ADPCM
  bytes; the compiled runtime's result-path probe resolves all 13 identities.
- User settings were restored after harness customization; no screenshots were
  generated. The game process exited cleanly. No five-hour soak was started.

Compact reports: `color-arcade-proof.json`, `color-quest-roster-proof.json`,
`staged-voice-proof.json`, and `final-proof.json` under
`artifacts/custom-color-result-voice-20260913/`. These tests prove the exercised
paths and source identity; user visual/listening review remains pending.
TO-DO #7 and #9 are pending review. No commit or push was requested.


## September 13 paint coverage and taillight follow-up

The remaining original-color body polygons were caused by initialization order:
the imported preview skipped the native pre-construction color-bank call. Meshes
therefore retained original polygon descriptors even though the later call
recolored a copied bank. Both native constructor calls now receive the imported
bank, including color zero, so the editable block exists before mesh binding.
Glass/reflection polygons are not forcibly tinted.

Taillights now use source-authored texture regions for both rosters. The offline
builder isolates their palette entries from body paint, promoting 4-bit textures
when necessary without changing a single default BGR555/STP texel. The shared
native recolor hook preserves the isolated entries using palette-content metadata;
there are no character-ID or map-specific runtime branches. This covers selector
and gameplay texture banks. Grayscale/unlit regions need no colored-lens split.

Rebuild with `python tools/recompone-v8-2/build_vehicle_light_masks.py --output <directory>`.
The embedded palette metadata and generated assets must be deployed together.
The generated native assets use pristine `V8_2_WORK/disc` inputs, never the
previously staged modified archives.

Validation:
- Houston selector front/rear: blue painted body, unchanged brown glass, trim and
  wheels, red taillights. Dallas rear: red lights with purple body; the pristine
  archive comparison recolors those lights purple. Side geometry and UI match.
- One continuous Arcade run exercised all 13 imported cars and the nine unlocked
  native IDs 0,1,2,6,7,8,12,13,14, with cancel/change/accept and repeat visits.
  All imported identities saved independently; clean process exit and no logged
  allocation or access violations. Locked native cars were not runtime-tested.
- Three light-mask tests pass, including every default native texture pixel in
  both archives; 15 conversion tests pass. The selector patch reapplies with
  zero changes. Visual checks are Houston and Dallas, not every roster member.
- Staged executable SHA256:
  `204daa7bbe57efc6ccd93ab0662ab9e5a1ede1542821b1f0cb26ec86cf8cd878`.
  Executable, native archives and guest package match test outputs byte-for-byte.

Evidence is in `artifacts/paint-mask-20260913/final-proof.json`, `full-roster/`,
`final-houston/`, `native-final/`, and `native-baseline/`. Houston and Dallas
screenshots were separately published in README at the user's request. Source
changes have not been committed as part of that documentation publication.
Victory VO (#9) was accepted by the user; it is no longer pending review.


The additional saved-color gameplay capture exposed a second ordering issue:
the body-bank color call preceded texture loading, so polygon RGB changed but
CLUT handles were not resident yet. Gameplay now repeats the native color pass
after object creation and VRAM placement, preserving the early descriptor copy.
This is the same two-phase native contract used by the selector, shared by all
registered vehicles. The first gameplay capture (yellow textures with blue body
polygons) is retained as a regression case in `gameplay-houston/`.

Final gameplay verification passed with executable `158bd68bf6db39a2ab6ecf296cf5ffaa858d092da88e8e9e24af776a51a5ed88`. The native capture
`artifacts/paint-mask-20260913/gameplay-fixed-houston/recompone_present_gameplay_0520_1824x1026_fxaa.png`
shows blue body textures and polygons, red taillights, black tires and neutral
trim/attachments after dismissing objectives. The process exited cleanly and
settings were restored. This final build supersedes the earlier selector-only
build hash above; unchanged selector behavior was covered by the preceding roster
run. All staged asset hashes remain unchanged. TO-DO #7 is PENDING REVIEW.


## Additional front/side lenses and Garbage Man taillights

User review identified amber and pale-red lenses missed by the earlier saturated
red filter. Added explicit fixed-lens regions, independent of hue/saturation,
for OG Beezwax, Loki, Dave, John Torque, Slick Clyde and Molo; native Nina Loco,
Molo and Garbage Man; native Clyde's amber fronts are covered as well. Both
Molo front headlamp assemblies are included. These masks cover selector and
matching gameplay/damaged texture variants and continue using the same shared
palette-content runtime metadata. No original default texel changes.

Four tests pass, including amber/pale-red colors shared with unmasked body
pixels, palette promotion and every native source pixel in both rebuilt archives.
The continuous selector run exited cleanly; front captures show Beezwax, Loki,
Dave, Torque and Molo amber lenses unchanged under purple recoloring. Nina's
side marker is visible. Clyde's front guard partially obscures the amber lenses;
source-mask verification supplies coverage for those obscured pixels. The first
Molo screenshot exposed unmasked neutral headlights; those were added before
the final build. Evidence: `artifacts/light-followup-20260913/`.

Final build `fafb549abb0fd70bec3490608eca6bf0d6c95faadb5f378be1678560fa40d566` is staged with matching native/guest assets.
The second targeted run exited cleanly, confirmed OG Molo side markers and
Garbage Man paired rear lamps retaining their colors under purple paint.
Settings were restored after both runs. #7 remains PENDING REVIEW.

User accepted and closed item #7 on September 13, 2026. Removed from the open-work tracker.
