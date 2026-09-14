# Soundtrack selection — 2026-09-10

Audio > Soundtrack offers **Vigilante 8**, **Second Offense**, and **Both**.
Vigilante 8 is the default for new settings and existing settings without the
field. The choice is saved immediately in settings.json. Switching albums starts
the first song and clears a pinned song index from the previous album.

The staged installation contains 12 original V8 songs and 16 Second Offense
songs; Both exposes all 28. `install_v8_soundtrack.py` copies the named original
OGGs from `PS1 game/music` and verifies their hashes without changing track02–17.
Older installations with no original music fall back to Second Offense while
retaining the saved preference.

## Implementation

- An audio-only TOC uses the actual OGG sample counts. The original data/file
  manifest stays authoritative for executable, overlay, model, XA, and movie
  reads. The combined audio timeline fits the native BCD clock (67.74 minutes).
- Native music setup/play seams use the playlist directly. They do not expand
  the retail TOC buffer or index the fifteen-entry name table beyond its end.
  Pause labels resolve original tracks by their source stem; Second Offense
  retains its retail titles where available.
- The existing native Random/Sequential/Loop modes retain their selection
  convention; the host streams and loops at exact CDDA sector boundaries.
  Song changes clear buffered audio under the same lock used by the CD reader.
- Continuous streaming now tracks its requested frame cursor instead of
  seeking whenever an OGG granule position differs. Full-track verification
  exposed that pre-existing issue in track10.ogg.
- Music/output/SFX controls remain on the native Audio page. Default gains
  were not retuned as part of the soundtrack option.

## Verification

Evidence directory: `artifacts/soundtrack-options-20260910/`.

Native visual run executable: `270612263574F360AB3E59C9C2B7F77A05EEBE09AB58F4C16A9A45D21855EBDA`.
Final staged executable: `4B627E3F717318A68A73526472ECD3CBE193BEDD2E99FF15F2C61697DC89418D`.
The intervening runtime source change only indented the new CDDA lock bodies.
Configuration save/reload was also tested before the final publish.

- `contract.log` / `pcm-proof.json`: all PCM samples of all 28 songs match
  independent sequential decoding of their sources at 44,100 Hz. No resampling,
  gain change, truncation, or slot-duration stretching. Final CD sectors may
  contain less than 588 frames of trailing zero padding.
- Runtime CDDA pump: the last song loops across leadout without stalling;
  switching to the original album clears queued audio and starts the new song.
- `config-contract.log`: all choices survive the actual runtime JSON
  save/reload path, retain the master volume, and normalize invalid values.
- `native-options/proof.json`: hidden native-menu run exits 0, switches through
  all three albums, saves Both, and plays track 28. Application-native captures
  at polls 120, 240, 360, and 660 were inspected individually: correct labels,
  counts, readable text, no overlap. The native mixed output is `audio.wav`.
- The harness restores the user's settings byte-for-byte. No desktop control,
  host input, performance campaign, or interactive capture was used.

The remaining #3 work is default music/voice/SFX balancing and audible user
acceptance. This implementation does not close that broader item.
