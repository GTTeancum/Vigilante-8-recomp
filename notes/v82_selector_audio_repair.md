# Character and AI carousel audio repair

The reported wrong sounds and pitches had two reproduced causes.

1. The converted shell bank kept all thirteen original-V8 selection voices
   resident in SPU memory. Its 357,424-byte payload plus the native
   286,848-byte BEGIN1 character bank exceeded the PS1's 512 KiB capacity.
   `SpuMallocPost` reset the allocator on entry to character selection and
   overwrote the still-live shell sounds. Those sounds then read character
   speech bytes using the old effects' addresses and pitches. The baseline
   log records the destructive reclaim immediately after `choose_player`.
2. Imported player speech was added when the enemy header was drawn, after
   native confirmation had already played the retail proxy's line. It also
   called the randomized, half-volume impact helper `8001E28C`, rather than
   the exact-pitch full-volume acceptance helper `8001E188`.

## Repair

`build_v8_selector_voice_bank.py` now emits a pair of original-format SND
files. `SHELL/SOUNDS.SND` contains the unchanged fourteen retail entries and
two reusable 32,928-byte slots. `SHELL/V8VOICES.SND` holds all thirteen
byte-exact imported samples and their original pitch fields outside SPU RAM.
The resident shell payload is 170,752 bytes. Together with BEGIN1 and the
allocator's initial reserved region, it needs 461,712 bytes and fits without
reducing sample rate, changing ADPCM, or enlarging emulated SPU memory.

At native player confirmation (`8010798C`), an imported identity replaces
the proxy line at that exact event. It uses the same native acceptance player, channel and volume as the
retail call, with the imported sample's authored pitch. Other callers and
stock characters continue through the original routine. In particular, the
AI-ready call at `801086CC` is a separate player-dialogue event, not a
voice announcement for the highlighted opponent; it remains native. The later header
draw no longer adds speech. The two resident slots follow native channels
1/2, so one channel's upload cannot replace another channel's bytes. Upload
and native key-on hold the SPU mixer lock together. The shell's existing
allocation/free lifecycle owns the slots; there are no extra SPU allocations
or persistent heap handles per selection.

Selector allocation failure can no longer reset the allocator under live
banks. Match-loading recovery remains separate. This guard is not the repair
for insufficient capacity: the resident-bank conversion removes the overflow.

The generated hook is preserved both by `prepare_reference.py` and the
idempotent native selector patch pass. Deployment must include the executable
and **both** SND files. Regenerate the pair with the original V8 SHELL.DLL and
SOUNDS.SND and either retail, legacy-expanded, or current resident V8:2
SOUNDS.SND as the builder inputs. No Dreamcast assets are involved.

## Evidence

- `artifacts/carousel-audio-baseline`: original failure, native mixed PCM,
  input fixture, and independently extracted retail sound bank.
- `artifacts/carousel-audio-fix/asset-contract.json`: 55 byte-exact asset and
  capacity checks. Independently rebuilding from original V8 sources also
  reproduces both staged files exactly. All thirteen source samples terminate
  with the original ADPCM end flag.
- Four converter tests cover retail preservation, roster-to-sample mapping,
  independent slots and idempotent rebuilding.
- Eighteen runtime assertions cover malformed sources, exact pitch/data,
  independent channel storage, atomic upload/start ordering, and preserving
  the live allocator on failure. Stock acceptance paths remain native.
- `artifacts/carousel-audio-stock-fixed`: stock navigation, confirmation and
  player-to-enemy handoff complete with no bank reclamation or allocation
  failure. `carousel.wav` is a trimmed native recording, not synthesized audio.
- `artifacts/carousel-audio-final-imported`: player acceptance produces a
  single key-on on native channel 2 at pitch `0x0514`, matching Y's authored
  pitch 1300. The earlier proxy line and randomized extra line are absent.
- `artifacts/carousel-audio-verified`: imported player acceptance and traversal
  of imported AI previews complete with clean exit and no live-bank reclaim.
  This covers AI navigation, not audible acceptance of every AI-ready dialogue.
- Final handoff `artifacts/carousel-audio-handoff/proof.json` passes: one imported
  acceptance key-on at the exact source pitch, enemy entry, cancellation back
  to the player carousel and then location selection, clean exit, nonzero
  native PCM, no allocator reclaim or failure. Settings restored byte-exactly.
  `selection.wav` is an unmodified seven-second excerpt of that native PCM.

Original repair handoff executable SHA256:
`DFC578A358D20165178A87CE40332EAEAB13425BE3BC2649E61BA27C402344D9`.

Audio is captured directly from the game's mixer into WAV while SDL uses a
silent dummy output device. Input remains inside the game process. These are
functional audio checks, not performance runs or human listening acceptance.

## September 11: two-player coverage — pending review

The staged executable tested was SHA256
`64c194bd4c8cc97f6a8e469b15734749985e1292ed380714c6aea597f476c94b`.
No runtime or audio-asset changes were made for this coverage.

`artifacts/carousel-audio-2p-reentry-20260911` contains the native mixer WAV,
runtime log, functional proof and waveform diagnostics. The process-local
fixture visits the simultaneous native two-player selector three times:
P1 confirms then cancels; P2 confirms then cancels; both confirm and reach
the ready prompt. P1 selects Chrono Stinger and P2 Dakota Stunt Cycle, since
the native menu rejects duplicate cars. Both native channels 3/4 (SPU voices
2/3) start at pitch 1300, matching all 18 entries in each BEGIN1/2/3 source
bank. No live-bank reclaim, selector allocation failure or fatal error was
logged. The process exits cleanly and settings are restored byte-for-byte.

This is native two-player coverage; it does not exercise imported character
menus in multiplayer. The previously accepted imported single-player and AI
coverage remains separate. The native two-player acceptance call at
`801086CC` uses channels 3/4; the imported single-player hook at `8010798C`
uses channels 1/2.

Waveform ranking identifies the held confirmations as the expected native
samples, but does not establish a full audible pass. The final confirmation
is cut short when the selector exits into setup/loading; native selector
teardown frees its preview banks. The diagnostic `two-player-audio.json`
remains failed under its full-phrase correlation threshold. That threshold
also rejects music-masked held speech, so it is not a substitute for listening.
The native exit behavior was preserved pending review, not patched with an
extra delay. Human listening and physical-controller acceptance remain open.

Unmodified mixer excerpts: `p1-confirmation.wav`, `p2-confirmation.wav`, and
`final-confirmation-transition.wav` in the same artifact directory.
