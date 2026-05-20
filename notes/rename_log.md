# Rename Log

Original address -> final name. Updated each decomp pass.

## Pass 1

### Hand-cleaned (HIGH/MED confidence)

| Address      | Original     | New name              | Confidence | Notes |
|--------------|--------------|-----------------------|------------|-------|
| 0x80044fbc   | FUN_80044fbc | `Heap_Init`           | HIGH       | K&R-style heap, 8-byte block header |
| 0x80045004   | FUN_80045004 | `Heap_Alloc`          | HIGH       | First-fit; was `malloc3` in PSY-Q |
| 0x80045088   | FUN_80045088 | `Heap_Free`           | HIGH       | Coalesces both neighbours |
| 0x80045134   | FUN_80045134 | `Heap_Realloc`        | HIGH       | Shrink in-place, else malloc+copy+free |
| 0x80044efc   | FUN_80044efc | `V8_MemSet`           | HIGH       | u32 unrolled body; misaligned head/tail |
| 0x80044c44   | FUN_80044c44 | `V8_MemCopy`          | HIGH       | 16-byte unrolled body |
| 0x80044d9c   | FUN_80044d9c | `V8_MemMove`          | HIGH       | Overlap-safe (direction chosen by ptr cmp) |
| 0x800154f4   | FUN_800154f4 | `V8_CdReadSectors`    | HIGH       | Sync read of 0x80 sectors at (buf, sector, mode) |
| 0x8001714c   | FUN_8001714c | `V8_SeedRng`          | MED        | Sets g_rngSeed + zeroes g_rngCounter. Consumer not yet identified. |

### Notable already-named (PSY-Q library entries)

These were named automatically by the PSX loader's signature DB and
will keep their PSY-Q names verbatim (HIGH per `DECOMP_RULES.md`):

`CdInit`, `CdRead`, `CdReadSync`, `CdControl`, `CdIntToPos`,
`SetDefDrawEnv`, `SetDefDispEnv`, `PutDispEnv`, `PutDrawEnv`,
`ClearOTagR`, `DrawOTag`, `DrawSync`, `OpenEvent`, `EnableEvent`,
`ResetGraph`, `SetGraphDebug`, `InitGeom`, `GetRCnt`, `SetRCnt`,
`VSyncCallback`, `PadInitDirect`, `MargePrim`, `SetDrawEnv`,
`ReadRotMatrix`, `SetRotMatrix`, `VectorNormalSS`,
and ~750 more (see `analysis/SLUS_005.10/symbols.txt`).

## Pending (pass 2 targets)

| Address      | Hypothesis            | Reason |
|--------------|-----------------------|--------|
| 0x80015098   | `V8_Main`             | Entry point's only call |
| 0x80013cac   | `V8_MainLoop`         | The 4932-byte function the main calls; contains the match loop |
| 0x80015e8c   | `V8_BootInit`         | Calls CdInit + heap setup |
| 0x80011adc   | `Asset_LoadDLL`       | Called with `"Shell\\Shell.dll"`; returns a handle whose +4 is a function pointer |
| 0x80044360   | `Asset_LoadSND`       | Called with `"Sounds\\Main.SND"` |
| 0x80015f80   | `Asset_LoadFNT`       | Called with `"Misc\\Game.FNT"` |
| 0x80015c68   | `Iso9660_ReadDir`     | Walks 14-byte entries from CD; name at +0x21 |
| 0x800156d4   | `V8_WaitVsync`        | Spin-wait on `iRam000006a8 != iRam000006a4` |
| 0x80043df8   | `Audio_PlayXa`        | Called with `"Sounds\\Victory.xa"` / `"Sounds\\Defeat.xa"` |
| 0x800251fc   | `V8_FrameTick(40)`    | Per-frame increment? Always called with 0x40 in the main loop |
| 0x80043ef0   | `Audio_Init` (?)      | Right before main loop starts |
| 0x80019034   | `Font_AllocAtlas` (?) | `(font, 0x23)` -- 0x23 might be slot count |

## Pass 1 stats

- Functions in main EXE: **1235**
- PSY-Q library auto-identified: **~801**
- Game-specific reachable from anchors: **291**
- Hand-cleaned: **9** (HIGH/MED above)
- Auto-stubbed (`src/<sub>/auto/*.c`): **277**
- Subsystems not yet touched: physics (all in TERRAIN DLLs)

## Pass 1 (continued)

Additional hand-cleaned (HIGH/MED):

| Address      | Original     | New name                    | File                          | Confidence |
|--------------|--------------|-----------------------------|-------------------------------|------------|
| 0x80011adc   | FUN_80011adc | `Overlay_LoadAndRelocate`   | src/assets/overlay_loader.c   | HIGH (PSY-Q overlay reloc format identified: tags 0/1/2/3 = abs32/HI16/abs16/J26) |
| 0x800128d4   | FUN_800128d4 | `Quest_Load`                | src/assets/quest_loader.c     | MED (record layout inferred from 16-byte stride + two pointer offsets) |
| 0x800156d4   | FUN_800156d4 | `V8_WaitVsync`              | src/gameplay/frame.c          | HIGH (canonical VSync IRQ spin) |

## Pass 1 (continued, second batch)

| Address      | Original     | New name                | File                       | Confidence |
|--------------|--------------|-------------------------|----------------------------|------------|
| 0x80015948   | FUN_80015948 | `Asset_LoadFile`        | src/assets/file_loader.c   | HIGH (alloc-sector-padded + CdRead + realloc-trim) |
| 0x800116f4   | FUN_800116f4 | `Heap_AllocOrRetry`     | src/assets/heap_retry.c    | HIGH (alloc + DrawSync + flush-free + retry-loop + _boot) |
| 0x8001570c   | FUN_8001570c | `Stream_Open`           | src/assets/cd_stream.c     | HIGH |
| 0x80015a20   | FUN_80015a20 | `Stream_Read`           | src/assets/cd_stream.c     | MED (unaligned fast-path simplified; pass 2 must verify bit-exact equivalence) |
| 0x80015be4   | FUN_80015be4 | `Stream_Tell`           | src/assets/cd_stream.c     | HIGH |
| 0x80015bf0   | FUN_80015bf0 | `Stream_Seek`           | src/assets/cd_stream.c     | HIGH |
| 0x80015a00   | FUN_80015a00 | `Stream_Close`          | src/assets/cd_stream.c     | MED |
| 0x800159b4   | FUN_800159b4 | `Stream_OpenByName`     | src/assets/cd_stream.c     | MED |
| 0x80044360   | FUN_80044360 | `Audio_LoadSND`         | src/assets/snd_loader.c    | HIGH (wrapper) |
| 0x80044394   | FUN_80044394 | `Audio_FreeSND`         | src/assets/snd_loader.c    | HIGH |
| 0x800451c0   | FUN_800451c0 | `Heap_Calloc`              | src/assets/heap_calloc.c   | HIGH |
| 0x80015798   | FUN_80015798 | `Stream_Close`             | src/assets/stream_close.c  | HIGH |
| 0x80015c68   | FUN_80015c68 | `Iso_ReadDir`              | src/assets/iso9660.c       | HIGH (ISO9660 record layout @ +0x00/0x19/0x20/0x21/0x02/0x0a confirmed) |
| 0x800157d4   | FUN_800157d4 | `Iso_OpenPath`             | src/assets/iso9660.c       | HIGH |
| 0x800523a0   | FUN_800523a0 | `Util_MemEq` (TBD)         | (still in auto/, pass 2)   | MED |
| 0x80044f64   | FUN_80044f64 | `V8_BZero`                  | src/assets/bzero.c            | HIGH |
| 0x80043aec   | FUN_80043aec | `CD_DetectDiskType`         | src/assets/cd_audio.c         | MED  |
| 0x80043bb4   | FUN_80043bb4 | `CD_IsCoverOpen`            | src/assets/cd_audio.c         | HIGH |
| 0x80043be4   | FUN_80043be4 | `CD_Pause`                  | src/assets/cd_audio.c         | HIGH |
| 0x80043c0c   | FUN_80043c0c | `CD_Play`                   | src/assets/cd_audio.c         | HIGH |
| 0x80043ce0   | FUN_80043ce0 | `CD_PlayTrack`              | src/assets/cd_audio.c         | MED  |
| 0x80043df8   | FUN_80043df8 | `Audio_PlayXa`              | src/assets/cd_audio.c         | MED  |
| 0x80043ef0   | FUN_80043ef0 | `Audio_Init` (OUT-OF-SCOPE) | src/skipped/audio_init.c      | HIGH (preserved as seam reference, gated #if 0) |
| 0x80043ff0   | FUN_80043ff0 | `Audio_KeyOnOff` (skipped)  | (covered in src/skipped/)     | -    |
| 0x80044054   | FUN_80044054 | `Audio_StopAll` (skipped)   | (covered in src/skipped/)     | -    |
| 0x80044080   | FUN_80044080 | `Audio_SetMasterVolume` (sk)| (covered in src/skipped/)     | -    |
| 0x800441f8   | FUN_800441f8 | `Audio_ParseSND`            | src/assets/snd_parse.c        | HIGH-MED (header u16 nSamples + u16 sizeIn8b confirmed; SPU 64-byte alignment preserved) |
| 0x800119c0   | FUN_800119c0 | `Buffer_StartOTag`          | src/gameplay/buffer.c         | HIGH |
| 0x80011914   | FUN_80011914 | `Buffer_FlushDeferredFree`  | src/gameplay/buffer.c         | HIGH |
| 0x800118b4   | FUN_800118b4 | `Buffer_DeferFree`          | src/gameplay/buffer.c         | HIGH |
| 0x8001178c   | FUN_8001178c | `Heap_CallocOrRetry`        | src/assets/heap_retry2.c      | HIGH |
| 0x80015610   | FUN_80015610 | `Iso_VerifyOriginalDisc`    | src/assets/iso_verify.c       | HIGH |
| 0x80043a74   | FUN_80043a74 | `Audio_RefreshKeyState` (renamed → `Audio_RefreshTOC`) | src/assets/cd_toc.c | HIGH |
| 0x800165cc   | FUN_800165cc | `Async_StopAllocCallback`   | src/assets/async_alloc.c      | HIGH |
| 0x80011c58   | FUN_80011c58 | `Match_LoadConfig`         | src/gameplay/match_config.c | HIGH (12-byte cfg layout confirmed) |
| 0x80011f8c   | FUN_80011f8c | `Player_GetType`           | src/gameplay/player_type.c  | MED  (name-char to type-int map; numeric labels unverified) |
| 0x8001265c   | FUN_8001265c | `IntroFlag_Reset`          | src/gameplay/misc_state.c   | HIGH |
| 0x800126c8   | FUN_800126c8 | `CharSelData_Free`         | src/gameplay/misc_state.c   | HIGH |
| 0x800126f0   | FUN_800126f0 | `Tick_PadOnly`             | src/gameplay/misc_state.c   | HIGH (single-line wrapper) |
| 0x80012088   | FUN_80012088 | `Util_BitSpread`           | src/gameplay/bit_interleave.c | HIGH (math) / MED (intended use) |
| 0x80020ad0   | FUN_80020ad0 | `Evict_LeafChainBackBuffer`| src/assets/asset_evict.c    | HIGH |
| 0x80020bec   | FUN_80020bec | `Evict_TreeWalk`           | src/assets/asset_evict.c    | HIGH-MED (kd-tree, kind 0/1/2) |
| 0x80020d3c   | FUN_80020d3c | `Asset_VisibilityEvictTick`| src/assets/asset_evict.c    | MED  |
| 0x8004316c   | FUN_8004316c | `GTE_RotateSV`             | src/physics/gte_rotate.c    | HIGH |
| 0x80043248   | FUN_80043248 | `GTE_RotateLong`           | src/physics/gte_rotate.c    | HIGH (17.15 split-int-frac transform) |
| 0x80043358   | FUN_80043358 | `GTE_RotateLongMat`        | src/physics/gte_rotate.c    | HIGH |
| 0x80043408   | FUN_80043408 | `GTE_RotateLongMatTrans`   | src/physics/gte_rotate.c    | HIGH |
| 0x8004352c   | FUN_8004352c | `GTE_RotateLongMtxLow`     | src/physics/gte_rotate.c    | HIGH (alias-ish) |
| 0x8004366c   | FUN_8004366c | `GTE_LoadMatrixPacked`     | src/physics/gte_rotate.c    | HIGH |
| 0x800436c8   | FUN_800436c8 | `GTE_LoadMatrixPackedAbs`  | src/physics/mat_abs_load.c  | HIGH |
| 0x80043754   | FUN_80043754 | `Vec5_ScaleI32`            | src/physics/vec_scale_pack.c| HIGH-MED |
| 0x80043864   | FUN_80043864 | `Vec5_ScaleI16`            | src/physics/vec_scale_pack.c| HIGH-MED |
| 0x800434d0   | FUN_800434d0 | `GTE_RotatePackedXYZ`      | (covered in gte_rotate.c)   | HIGH |
| 0x80043974   | FUN_80043974 | `GTE_LoadTransform`        | src/physics/gte_load_xform.c | HIGH |
| 0x80011a10   | FUN_80011a10 | `Buffer_StartOTagOther`    | src/gameplay/tick_misc.c    | HIGH |
| 0x80012028   | FUN_80012028 | `MatchSlot_SetAll`         | src/gameplay/match_slots.c  | MED  |
| 0x80012050   | FUN_80012050 | `MatchSlot_SetCharacter`   | src/gameplay/match_slots.c  | MED  |
| 0x80012068   | FUN_80012068 | `MatchSlot_SetTrim`        | src/gameplay/match_slots.c  | MED  |
| 0x80015010   | FUN_80015010 | `Tick_GetFrameCounter`     | src/gameplay/tick_misc.c    | HIGH |
| 0x800128bc   | FUN_800128bc | `Tick_WaitIrqAck`          | src/gameplay/tick_misc.c    | HIGH |
| 0x80012980   | FUN_80012980 | `Quest_Free`               | src/gameplay/tick_misc.c    | HIGH |
| 0x8001555c   | FUN_8001555c | `Iso_ReadPVD`              | src/assets/iso_pvd.c        | HIGH (sector 0x10, first 32 B copied) |
| 0x80015164   | FUN_80015164 | `Splash_InitDisplay`       | src/gameplay/splash.c       | MED  |
| 0x80015288   | FUN_80015288 | `Splash_DrawLine`          | src/gameplay/splash.c       | MED  |
| 0x80015368   | FUN_80015368 | `Stream_FatalOom`          | src/gameplay/splash.c       | HIGH |
| 0x800116ec   | FUN_800116ec | `Bootstrap_C_Init` (stub)  | src/gameplay/stub_misc.c    | HIGH (empty function) |
| 0x80040e18   | FUN_80040e18 | `FreeHelper_80040e18`      | src/gameplay/stub_misc.c    | HIGH |
| 0x8003d898   | FUN_8003d898 | `ScreenHalf_Configure`     | src/gameplay/stub_misc.c    | HIGH |
| 0x80016c54   | FUN_80016c54 | `Math_Atan2_PosNeg`        | src/physics/math_helpers.c  | MED  |
| 0x80016c88   | FUN_80016c88 | `Math_Atan2_Pos`           | src/physics/math_helpers.c  | MED  |
| 0x80016dfc   | FUN_80016dfc | `Matrix_InverseRigid`      | src/physics/math_helpers.c  | HIGH |
| 0x80016e64   | FUN_80016e64 | `Math_NormalizeXZ`         | src/physics/math_helpers.c  | HIGH-MED |
| 0x80022524   | FUN_80022524 | `Iff_ReadChunkHeader`      | src/assets/iff_chunk.c      | HIGH (EXP/IFF chunk parser core) |
| 0x80015e8c   | FUN_80015e8c | `Boot_Init`                | src/assets/boot_init.c       | HIGH (CdInit + read PVD + cache + ISO9660 root walk) |
| 0x80015f80   | FUN_80015f80 | `Asset_LoadFileOrDie`      | src/assets/asset_load_ordie.c | HIGH |
| 0x80016024   | FUN_80016024 | `Render_BuildLogoSurface`  | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x80016678   | FUN_80016678 | `Async_StartLogo`          | src/assets/asyncalloc_begin.c | HIGH (matches StopAlloc teardown sequence) |
| 0x80011834   | FUN_80011834 | `AsyncList_RecycleHead`    | src/assets/async_list_recycle.c | HIGH-MED |
| 0x800225d4   | FUN_800225d4 | `Iff_ReadChunkData`        | src/assets/iff_chunk_data.c  | HIGH (header + alloc + payload-stream) |
| 0x80022a1c   | FUN_80022a1c | `Level_Teardown`           | src/gameplay/level_teardown.c | MED (partial -- a few trailing chain detaches deferred to pass 2) |
| 0x80022cd0   | FUN_80022cd0 | `Render_DrawScaledModel`   | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x80025b20   | FUN_80025b20 | `Tri_InterpEdge`           | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x80025bc0   | FUN_80025bc0 | `Tri_ClipNear`             | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x80025400   | FUN_80025400 | `Terrain_HeightAt`         | src/physics/terrain_height.c | HIGH (32x32 of 64x64x u16 heightmap with bilinear blend) |
| 0x800255f4   | FUN_800255f4 | `Terrain_MaterialAt`       | src/physics/terrain_height.c | HIGH |
| 0x800251fc   | FUN_800251fc | `Terrain_InitFlatWorld`    | src/physics/terrain_init.c   | MED  |
| 0x800220d4   | FUN_800220d4 | `Terrain_IsMostlyResident` | src/physics/terrain_visibility.c | MED |
| 0x8003e2c4   | FUN_8003e2c4 | `HandlePair_Free`          | src/gameplay/handle_pair_free.c | HIGH |
| 0x80025800   | FUN_80025800 | `Terrain_NormalAt`         | (deferred to pass 2)         | UNKNOWN -- large; pruned for now |
| 0x8002623c+  | FUN_8002623c | recursive grid visit       | (deferred to pass 2)         | UNKNOWN |
| 0x800288fc+  | FUN_800288fc | grid-range bit scan        | (deferred to pass 2)         | UNKNOWN |
| 0x800290d8   | FUN_800290d8 | uses scratchpad (RFU)      | (deferred to pass 2)         | UNKNOWN |
| 0x8002accc   | FUN_8002accc | Render_Free_5Buffers       | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x8002ad30   | FUN_8002ad30 | Render_MinimapBlips        | (renderer, doc only)         | OUT-OF-SCOPE |
| 0x8002263c   | FUN_8002263c | shell-UI text builder      | (UI, doc only)               | OUT-OF-SCOPE |
| 0x80015098   | FUN_80015098 | `V8_Main`                  | src/gameplay/main_entry.c    | HIGH (full cold-boot sequence) |
| 0x80013cac   | FUN_80013cac | `V8_MainLoop`              | (deferred -- see src/gameplay/pass2_pending.md) | PASS2 |
| 0x800120d4   | FUN_800120d4 | `Pad_Tick`                 | (deferred -- pass 2 + out-of-scope per CLAUDE.md) | PASS2 |
| 0x80012a90   | FUN_80012a90 | `Menu_Pause`               | (deferred) | PASS2 |
| 0x8001356c   | FUN_8001356c | `MainLoop_FinalizeMatch`   | (deferred) | PASS2 |
| 0x800136c4   | FUN_800136c4 | `MainLoop_TransitionToShell` | (deferred) | PASS2 |
| 0x8001392c   | FUN_8001392c | `MainLoop_StartMatch`      | (deferred) | PASS2 |
| 0x8003e2fc   | FUN_8003e2fc | `Camera_BuildMatrix`       | (deferred) | PASS2 |

## DLL Overlay decomp (pass 1, starting)

| DLL          | Address     | Original     | New name              | File                                  | Confidence |
|--------------|-------------|--------------|-----------------------|---------------------------------------|------------|
| AIRGRAVE.DLL | 0x8010068c  | FUN_8010068c | `AGTracker_Tick`      | src/physics/airgrave/tracker.c        | MED  (per-tick homing/firing of AirGrave crane + B-17) |

## Pass 2 (in progress)

| Address      | Original     | New name              | File                          | Confidence | Notes |
|--------------|--------------|-----------------------|-------------------------------|------------|-------|
| 0x8002263c   | FUN_8002263c | `XOBF_Parse`          | src/assets/xobf_parse.c       | HIGH | walks BIN / ANM / SND sub-chunks; builds object+chain |
| (SHELL.DLL)  | FUN_80102264 | `VehicleExp_Load`     | src/assets/vehicle_exp.c      | HIGH | loads Shell\Vehicles.exp into DAT_800737a0[] |
| 0x8001a640   | FUN_8001a640 | `Renderer_BuildObject`| (renderer seam, doc only)     | MED  | object handle factory; pass-2 stub |
| 0x8001a91c   | FUN_8001a91c | `DrawChain_Register`  | (renderer seam, doc only)     | MED  |  |

## Pass 2 (LOAD.DLL + SHELL.DLL helpers)

| Addr (DLL)         | Original     | New name              | File                          | Confidence |
|--------------------|--------------|-----------------------|-------------------------------|------------|
| 0x80102334 (SHELL) | FUN_80102334 | `VehicleExp_Free`     | src/assets/vehicle_exp.c      | HIGH (walks DAT_800737a0[0..0xe], frees) |
| 0x80100408 (LOAD)  | FUN_80100408 | `XOBF_AppendExtra`    | src/assets/xobf_helpers.c     | HIGH |
| 0x801005e8 (LOAD)  | FUN_801005e8 | `XOBF_LoadBlob`       | src/assets/xobf_helpers.c     | HIGH |
| 0x80100594 (LOAD)  | FUN_80100594 | `XOBF_LoadFNT`        | src/assets/xobf_helpers.c     | HIGH |
| 0x801005c0 (LOAD)  | FUN_801005c0 | `XOBF_StashReturn`    | src/assets/xobf_helpers.c     | HIGH |
| 0x8010243c (LOAD)  | FUN_8010243c | `XOBF_AllocArray`     | src/assets/xobf_helpers.c     | HIGH |
| 0x80100e20 (LOAD)  | FUN_80100e20 | `XOBF_ReadCollisionBox` | src/assets/xobf_helpers.c   | HIGH (per-object AABB layout recovered) |
| 0x80102174 (SHELL) | FUN_80102174 | `ShellEvent_Empty`    | src/gameplay/shell_misc.c     | HIGH |
| 0x801021a0 (SHELL) | FUN_801021a0 | `ShellEvent_Empty2`   | src/gameplay/shell_misc.c     | HIGH |
| 0x8010754c (SHELL) | FUN_8010754c | `ShellEvent_Empty3`   | src/gameplay/shell_misc.c     | HIGH |
| 0x801021d4 (SHELL) | FUN_801021d4 | `Shell_WaitForStartPress` | src/gameplay/shell_misc.c | MED  |
| 0x8010223c (SHELL) | FUN_8010223c | `Shell_RealignCursor` | src/gameplay/shell_misc.c     | HIGH |
| 0x80100e98 (LOAD)  | FUN_80100e98 | `Iff_DiscardChunks`   | src/assets/iff_discard.c      | HIGH |
| 0x80101958 (SHELL) | FUN_80101958 | `Cheat_Check`         | src/gameplay/cheat_code.c     | HIGH (14-byte PRNG scramble against DAT_801122bc table) |
| 0x80101d6c (SHELL) | FUN_80101d6c | `ShellEvent_Empty4`   | src/gameplay/shell_misc.c     | HIGH |
| 0x801001ec (LOAD)  | FUN_801001ec | `IffNode_Link`        | src/assets/load_helpers.c     | HIGH |
| 0x8010063c (LOAD)  | FUN_8010063c | `Map_ReadRegion`      | src/assets/load_helpers.c     | HIGH (14-byte map region layout) |
| 0x801061c4 (LOAD)  | FUN_801061c4 | `DrawPrim_SetColor`   | src/assets/load_helpers.c     | HIGH |
| 0x801064ac (LOAD)  | FUN_801064ac | `DMA_RegisterCallback0` | src/assets/load_helpers.c    | HIGH |
| 0x801064d0 (LOAD)  | FUN_801064d0 | `DMA_RegisterCallback1` | src/assets/load_helpers.c    | HIGH |
| 0x80106414 (LOAD)  | FUN_80106414 | `MDEC_Empty2`         | src/assets/load_helpers.c     | HIGH |
| 0x801060ac (LOAD)  | FUN_801060ac | `MDEC_Empty0`         | src/assets/load_helpers.c     | HIGH |
| 0x801060d0 (LOAD)  | FUN_801060d0 | `MDEC_Empty1`         | src/assets/load_helpers.c     | HIGH |
| 0x80106434 (LOAD)  | FUN_80106434 | `MDEC_StatusBit_29`   | src/assets/load_helpers.c     | HIGH |
| 0x80106470 (LOAD)  | FUN_80106470 | `MDEC_StatusBit_24`   | src/assets/load_helpers.c     | HIGH |
| 0x80104c68 (LOAD)  | FUN_80104c68 | `Terrain_ColorAt`     | src/physics/terrain_color.c   | HIGH-MED (height-keyed greyscale RGB) |
| 0x8010059c (AIRGR) | FUN_8010059c | `AGProjectile_Tick`   | src/physics/airgrave/projectile.c | HIGH (gravity + terrain-collide; gravity const 0x38, forward 0x1dcd) |
| 0x80100be8 (CANYNLND) | FUN_80100be8 | `Boulder_Tick`     | src/physics/canynlnd/boulder.c   | MED |
| 0x80100c6c (SANDFACT) | FUN_80100c6c | `SF_ConveyorTick`  | src/physics/sandfact/conveyor.c  | HIGH-MED |
| 0x801004a8 (SCRTBASE) | FUN_801004a8 | `SCRT_RadarTick`   | src/physics/scrtbase/radar.c     | MED |
| 0x80017160 (main)  | FUN_80017160 | `V8_RandNext`         | src/gameplay/rng.c            | HIGH (bit-exact xorshift; consumer side of V8_SeedRng) |
| 0x801006cc (HOOVRDAM) | FUN_801006cc | `HD_DamLeverTick`  | src/physics/hoovrdam/dam_lever.c | MED |
| 0x8010020c (CASNOCTY) | FUN_8010020c | `CC_BlimpTick`      | src/physics/casnocty/blimp.c   | MED |
| 0x80101050 (SKIRESRT) | FUN_80101050 | `SR_GondolaTick`   | src/physics/skiresrt/gondola.c   | MED |
| 0x801005e8 (VALLYFRM) | FUN_801005e8 | `VF_WindmillTick`  | src/physics/vallyfrm/windmill.c  | MED |
| 0x801003ec (WILDWEST) | FUN_801003ec | `WW_BridgeDestroyTick`| src/physics/wildwest/bridge.c    | MED |
| 0x8010047c (OILFIELD) | FUN_8010047c | `OF_ProjectileSpawn`  | src/physics/oilfield/projectile.c | MED |
| 0x80017324 (main)  | FUN_80017324 | `Object_IntegrateAndOrient` | src/physics/object_integrate.c | HIGH (core per-frame integrator for all movable objects) |
| 0x800439b8 (main)  | FUN_800439b8 | `Object_ApplyAngularVelocity` | src/physics/object_integrate.c | HIGH-MED (column-1 promoted; pass 3 finishes 2/3) |
| 0x800205f8 (main)  | FUN_800205f8 | `Damage_Apply`        | src/gameplay/damage_apply.c   | HIGH (entry-point dispatcher) |
| 0x8002179c (main)  | FUN_8002179c | `Damage_RouteByTree`  | src/gameplay/damage_apply.c   | HIGH |
| 0x800205a0 (main)  | FUN_800205a0 | `Object_RetireToDeadList` | src/gameplay/damage_apply.c | HIGH |
| 0x80020778 (main)  | FUN_80020778 | `Object_ClearBackBufferFlag` | src/gameplay/damage_apply.c | HIGH |
| 0x8001fe8c (main)  | FUN_8001fe8c | `ObjList_RemoveByPayload` | src/gameplay/object_list.c | HIGH |
| 0x8001ff58 (main)  | FUN_8001ff58 | `ObjList_FindBySpawnId`   | src/gameplay/object_list.c | HIGH |
| 0x8001ff0c (main)  | FUN_8001ff0c | `ObjList_FindByPayload`   | src/gameplay/object_list.c | HIGH |
| 0x800210a4 (main)  | FUN_800210a4 | `ObjList_RemoveFromTree`  | src/gameplay/object_list.c | HIGH |
| 0x80020540 (main)  | FUN_80020540 | `Object_Free`             | src/gameplay/object_list.c | HIGH |
| 0x800204dc (main)  | FUN_800204dc | `Tree_Free`              | src/gameplay/object_tree.c | HIGH |
| 0x8002123c (main)  | FUN_8002123c | `Tree_Apply`             | src/gameplay/object_tree.c | HIGH |
| 0x8002131c (main)  | FUN_8002131c | `ObjList_TickAll`        | src/gameplay/object_tree.c | HIGH |
| 0x800212c4 (main)  | FUN_800212c4 | `ObjList_PreTickAll`     | src/gameplay/object_tree.c | HIGH |
| 0x80021394 (main)  | FUN_80021394 | `TriggerVol_ExpireFromHead` | src/gameplay/object_tree.c | MED |
| 0x8001fcb4 (main)  | FUN_8001fcb4 | `Object_FrameCounterBump`     | src/gameplay/list_walkers.c | HIGH |
| 0x80020658 (main)  | FUN_80020658 | `ObjList_FreeAllAndRetire`    | src/gameplay/list_walkers.c | HIGH |
| 0x80020968 (main)  | FUN_80020968 | `Tree_FreeRecursive`          | src/gameplay/list_walkers.c | HIGH |
| 0x800206f0 (main)  | FUN_800206f0 | `ObjList_ApplyDestroy`        | src/gameplay/list_walkers.c | HIGH |
| 0x800200b8 (main)  | FUN_800200b8 | `ObjList_FindWithPredicate`   | src/gameplay/list_walkers.c | HIGH |
| 0x8001bddc (main)  | FUN_8001bddc | `Object_FreeAndUnhook` | src/gameplay/object_teardown.c | HIGH |
| 0x8001db54 (main)  | FUN_8001db54 | `Culling_SphereInsideFrustum`| src/physics/culling.c        | HIGH |
| 0x8001d624 (main)  | FUN_8001d624 | `Matrix_ComposeParentChain`  | src/physics/matrix_chain.c   | HIGH |
| 0x8001d9c0 (main)  | FUN_8001d9c0 | `Camera_ApplyFrame`          | src/physics/camera_apply.c   | HIGH |
| 0x8001db24 (main)  | FUN_8001db24 | `Camera_AttachToObject`      | src/physics/camera_apply.c   | HIGH |
| 0x80022ba8 (main)  | FUN_80022ba8 | `Level_LoadViaShellLoadDll`  | src/gameplay/level_load.c    | HIGH |
| 0x80029dec (main)  | FUN_80029dec | `Debris_Pool_InitFrame`      | src/gameplay/debris_pool_init.c | HIGH |
| 0x801002ac (LOAD)  | FUN_801002ac | `Audio_LoadTrackTable`| src/assets/load_track_table.c | MED  |
| 0x801016b4 (SHELL) | FUN_801016b4 | `Cheat_DecodeToAscii` | src/gameplay/cheat_decode.c   | MED  |
| 0x8010238c (SHELL) | FUN_8010238c | `Slogan_Show`         | src/gameplay/slogan_screen.c  | HIGH-MED (slogan random pick: `(rand * 11) >> 15`) |
| 0x8010036c (HOOVRDAM) | FUN_8010036c | `HD_TransformerCleanup`| src/physics/hoovrdam/misc.c | MED |
| 0x801013e0 (HOOVRDAM) | FUN_801013e0 | `HD_TransformerDestroy`| src/physics/hoovrdam/misc.c | MED |
| 0x8010135c (HOOVRDAM) | FUN_8010135c | `HD_PipeOutDestroy`    | src/physics/hoovrdam/misc.c | MED |
| 0x80101118 (HOOVRDAM) | FUN_80101118 | `HD_ArchTransfImpact`  | src/physics/hoovrdam/misc.c | MED |
| 0x801006f4 (SCRTBASE) | FUN_801006f4 | `SB_FenceImpact`      | src/physics/scrtbase/misc.c   | HIGH-MED |
| 0x801025ec (SCRTBASE) | FUN_801025ec | `SB_TurretHit`        | src/physics/scrtbase/misc.c   | MED  |
| 0x80101ca8 (CASNOCTY) | FUN_80101ca8 | `CC_GenericCleanup`   | src/physics/casnocty/misc.c   | MED  |
| 0x80100168 (CASNOCTY) | FUN_80100168 | `CC_BurgerSignReset`  | src/physics/casnocty/misc.c   | MED  |
| 0x80100950 (WILDWEST) | FUN_80100950 | `WW_BonfireImpact`    | src/physics/wildwest/misc.c   | MED |
| 0x8010035c (WILDWEST) | FUN_8010035c | `WW_HotelHit`         | src/physics/wildwest/misc.c   | MED |
| 0x80100c50 (CANYNLND) | FUN_80100c50 | `CL_BeamTick`         | src/physics/canynlnd/beam.c     | HIGH-MED |
| 0x801010c4 (SANDFACT) | FUN_801010c4 | `SF_TerrainOnlyImpact`| src/physics/sandfact/factory_collide.c | HIGH |
| 0x80101424 (SKIRESRT) | FUN_80101424 | `SR_InstallDefaultTick`| src/physics/skiresrt/misc.c   | HIGH |
| 0x80101390 (SKIRESRT) | FUN_80101390 | `SR_InsertSortedByY`  | src/physics/skiresrt/misc.c   | MED  |
| 0x80102094 (SKIRESRT) | FUN_80102094 | `SR_SnowMachineHit`   | src/physics/skiresrt/misc.c   | MED  |
| 0x801002c4 (VALLYFRM) | FUN_801002c4 | `VF_GenericCleanup`   | src/physics/vallyfrm/silo_cleanup.c | HIGH |
| 0x801001cc (OILFIELD) | FUN_801001cc | `OF_RigInit`          | src/physics/oilfield/rig_init.c | MED |
| 0x80100c18 (AIRGRAVE) | FUN_80100c18 | `AG_ControlTowerImpact`| src/physics/airgrave/control_tower.c | HIGH |
| 0x80100624 (SCRTBASE) | FUN_80100624 | `SB_CatwalkBounds`    | src/physics/scrtbase/catwalk.c | HIGH-MED |
| 0x80100bd0 (CASNOCTY) | FUN_80100bd0 | `CC_PalmSpawn`        | src/physics/casnocty/palm_spawn.c | MED |
| 0x80100540 (OILFIELD) | FUN_80100540 | `OF_SphereTick`       | src/physics/oilfield/sphere.c | MED |
| 0x8001a640 (main)  | FUN_8001a640 | `Object_BuildFromBin` | src/gameplay/object_lifecycle.c | MED-HIGH (closes Renderer_BuildObject from XOBF_Parse) |
| 0x8001a994 (main)  | FUN_8001a994 | `Object_FinishBuild`  | src/gameplay/object_lifecycle.c | HIGH |
| 0x8001a91c (main)  | FUN_8001a91c | `Object_RegisterInChain` | src/gameplay/object_lifecycle.c | HIGH |
| 0x8001aa38 (main)  | FUN_8001aa38 | `Object_FreeAndChildren` | src/gameplay/object_lifecycle.c | HIGH |
| 0x8001aa0c (main)  | FUN_8001aa0c | `Object_FreeWithoutSound` | src/gameplay/object_lifecycle.c | HIGH |
| 0x8001b07c (main)  | FUN_8001b07c | `Matrix_FromObjectTransform` | src/physics/matrix_from_obj.c | HIGH |
| 0x8004410c (main)  | FUN_8004410c | `Audio_VoiceAlloc` (skipped)| src/skipped/audio_voice.c     | doc-only |
| 0x800443c8 (main)  | FUN_800443c8 | `Audio_PlaySfx_inner` (sk)  | src/skipped/audio_voice.c     | doc-only |
| 0x800441c8 (main)  | FUN_800441c8 | `Audio_VoiceStop` (sk)      | src/skipped/audio_voice.c     | doc-only |
| 0x80044054 (main)  | FUN_80044054 | `Audio_StopAll` (sk)        | src/skipped/audio_voice.c     | doc-only |
| 0x80044574 (main)  | FUN_80044574 | `SPU_VoiceVolume_Set` (sk)  | src/skipped/audio_voice.c     | doc-only |
| 0x800449bc (main)  | FUN_800449bc | `SfxPan_For3DPos`     | src/physics/sound_position.c  | MED-HIGH |
| 0x800446dc (main)  | FUN_800446dc | `SfxPan_For3DPosDelayed` | src/physics/sound_position.c | MED-HIGH |
| 0x8003fc50 (main)  | FUN_8003fc50 | `Effects_SpawnExplosion`| src/physics/effects.c       | MED |
| 0x8003fc94 (main)  | FUN_8003fc94 | `Effects_QueueSnow`     | src/physics/effects.c       | MED |
| 0x8003fd24 (main)  | FUN_8003fd24 | `Effects_SpawnParticleAtParent` | src/physics/effects.c | MED |
| 0x800523a0 (main)  | FUN_800523a0 | `Util_MemEq`          | src/assets/memcmp.c           | HIGH (standard PSY-Q memcmp) |
| 0x80022320 (main)  | FUN_80022320 | `Damage_AccumulateOrFire` | src/gameplay/collision_apply.c | HIGH (the universal damage arbiter; obj+0xc = health, +0xe = maxHealth) |
| 0x8002239c (main)  | FUN_8002239c | `Damage_FromImpulse`  | src/gameplay/collision_apply.c | HIGH |
| 0x80043224 (main)  | FUN_80043224 | `GTE_GetCurrentPos`   | src/physics/gte_pos.c         | HIGH |
| 0x8002185c (main)  | FUN_8002185c | `Object_BroadcastToTree`| src/gameplay/object_broadcast.c | HIGH |
| 0x80021924 (main)  | FUN_80021924 | `Object_BroadcastWorldOrTree` | src/gameplay/object_broadcast.c | HIGH |
| 0x80022c54 (main)  | FUN_80022c54 | `Object_AppendToScratchList` | src/gameplay/object_broadcast.c | HIGH |
| 0x8001fd9c (main)  | FUN_8001fd9c | `Object_FindByIdPlusOffset` | src/gameplay/object_findbyid.c | HIGH |
| 0x80052544 (main)  | FUN_80052544 | `Util_StrLen`         | src/assets/strlen.c           | HIGH |
| 0x80024718 (main)  | FUN_80024718 | `Object_SetState`     | src/gameplay/object_state.c   | HIGH |
| 0x80021888 (main)  | FUN_80021888 | `Object_BroadcastEventWorld` | src/gameplay/object_state.c | HIGH |
| 0x80100940 (CANYNLND) | FUN_80100940 | `CL_Launcher`         | src/physics/canynlnd/spawner.c | MED |
| 0x80101a98 (HOOVRDAM) | FUN_80101a98 | `HD_TransfBoxBroadcast`| src/physics/hoovrdam/misc2.c | MED |
| 0x80100b40 (HOOVRDAM) | FUN_80100b40 | `HD_PipeChildSpawn`   | src/physics/hoovrdam/misc2.c   | MED |
| 0x801019d8 (SKIRESRT) | FUN_801019d8 | `SR_BallDestroy`      | src/physics/skiresrt/ball.c    | MED |
| 0x80022e90 (main)  | FUN_80022e90 | `Vehicle_TryAcquireTarget` | src/gameplay/ai_target.c | MED |
| 0x8001d564 (main)  | FUN_8001d564 | `Object_DetachFromParent` | src/gameplay/object_hierarchy.c | HIGH |
| 0x8001d5a0 (main)  | FUN_8001d5a0 | `Object_Parent`           | src/gameplay/object_hierarchy.c | HIGH |
| 0x80016da8 (main)  | FUN_80016da8 | `Matrix_PackedIdentity` | src/physics/matrix_identity.c | HIGH |
| 0x80020000 (main)  | FUN_80020000 | `ObjList_FindBySpawnIdValue` | src/gameplay/object_list_extra.c | HIGH |
| 0x8002002c (main)  | FUN_8002002c | `ObjList_TickListWith3Args` | src/gameplay/object_list_extra.c | HIGH |
| 0x80020120 (main)  | FUN_80020120 | `ObjList_CountWithFlag`   | src/gameplay/object_list_extra.c | HIGH |
| 0x80020190 (main)  | FUN_80020190 | `ObjList_NthWithFlag`     | src/gameplay/object_list_extra.c | HIGH |
| 0x800202f4 (main)  | FUN_800202f4 | `Object_RegisterInScene`  | src/gameplay/object_list_extra.c | HIGH |
| 0x8001fe50 (main)  | FUN_8001fe50 | `ObjList_FastInsert`       | src/gameplay/object_post_update.c | HIGH |
| 0x80020744 (main)  | FUN_80020744 | `Object_RegisterPostUpdate`| src/gameplay/object_post_update.c | HIGH |
| 0x8003eab0 (main)  | FUN_8003eab0 | `Projectile_GravityTick`   | src/gameplay/object_post_update.c | HIGH-MED |
| 0x8001d748 (main)  | FUN_8001d748 | `Terrain_HeightAndProbe` | src/physics/terrain_probe.c   | HIGH (combined terrain + obstacle-chain height probe) |
| 0x80025648 (main)  | FUN_80025648 | `Terrain_NormalAt`    | src/physics/terrain_normal.c  | HIGH-MED |
| 0x8001a4f8 (main)  | FUN_8001a4f8 | `Layout_PushClip` (renderer seam) | (src/skipped doc-only) | doc-only |
| 0x8001a2cc (main)  | FUN_8001a2cc | `Layout_BeginCenter` (renderer seam) | (src/skipped doc-only) | doc-only |
| 0x80019e20 (main)  | FUN_80019e20 | `Render_ResetDrawEnv_640` (renderer) | (src/skipped doc-only) | doc-only |
| 0x80019e7c (main)  | FUN_80019e7c | `Render_SetVideoMode` (renderer) | (src/skipped doc-only) | doc-only |
| 0x80042724 (main)  | FUN_80042724 | `Particle_TileFit` (renderer geom) | (src/skipped doc-only) | doc-only |
| 0x80042cdc (main)  | FUN_80042cdc | `Particle_NearestSlot` (renderer alloc) | (src/skipped doc-only) | doc-only |
| 0x800422d8 (main)  | FUN_800422d8 | `Proximity_DispatchInRange` | src/gameplay/proximity_dispatch.c | HIGH |
| 0x8001afa0 (main)  | FUN_8001afa0 | `Bone_FindByKind`    | src/gameplay/bone_lookup.c    | HIGH |
| 0x8001e120 (main)  | FUN_8001e120 | `Object_TickCallback`  | src/gameplay/callback_dispatch.c | HIGH |
| 0x80014ff0 (main)  | FUN_80014ff0 | `V8_TimerIRQ`         | src/gameplay/timer_irq.c      | HIGH (the OpenEvent RCnt2 handler) |
| 0x800166dc (main)  | FUN_800166dc | `ObjList_Length`      | src/gameplay/objlist_count.c  | HIGH |
| 0x800118b4 (main)  | FUN_800118b4 | `Buffer_DeferFree`    | src/gameplay/buffer_defer_free.c | HIGH (was previously stubbed in gameplay/buffer.c; consolidated) |
| 0x801014d0 (AIRGRAVE) | FUN_801014d0 | `AG_B17HitTick`       | src/physics/airgrave/b17_hit.c | MED |
| 0x80101220 (VALLYFRM) | FUN_80101220 | `VF_PumpTick`         | src/physics/vallyfrm/pump.c   | MED |
| 0x80100a9c (CASNOCTY) | FUN_80100a9c | `CC_RandomFire`       | src/physics/casnocty/spawner.c | MED |
| 0x8003fbc8 (main)  | FUN_8003fbc8 | `Bone_FindFreeSlot`   | src/gameplay/bone_findfree.c  | HIGH |
| 0xc784 (SHELL.DLL entry) | FUN_8010c784 | `Shell_TitleStateMachine` | (deferred -- 2K bytes, complex state machine) | PASS3 |
| 0x801003f4 (SCRTBASE) | FUN_801003f4 | `SB_TurretSfxTick`    | src/physics/scrtbase/turret_sfx.c | MED |
| 0x80101a90 (CASNOCTY) | FUN_80101a90 | `CC_RandomScatterTick`| src/physics/casnocty/spawner2.c | MED |
| 0x80101bb8 (CASNOCTY) | FUN_80101bb8 | `CC_BoneSpawn`        | src/physics/casnocty/spawner2.c | MED |
| 0x801005e4 (WILDWEST) | FUN_801005e4 | `WW_ShackTick`        | src/physics/wildwest/shack.c   | MED |
| 0x80100b34 (SANDFACT) | FUN_80100b34 | `SF_ElevatorTick`     | src/physics/sandfact/elevator.c | MED |
| 0x80100870 (OILFIELD) | FUN_80100870 | `OF_PipeEndSpawn`     | src/physics/oilfield/pipe_end.c | MED |
| 0x80100a18 (HOOVRDAM) | FUN_80100a18 | `HD_ScatterTick`      | src/physics/hoovrdam/scatter.c | MED |
| 0x80101284 (SKIRESRT) | FUN_80101284 | `SR_LiftStation`      | src/physics/skiresrt/lift.c   | MED |
| 0x801004cc (WILDWEST) | FUN_801004cc | `WW_TrainInit`        | src/physics/wildwest/train.c   | MED |
| 0x80100cbc (CANYNLND) | FUN_80100cbc | `CL_BoulderChain`     | src/physics/canynlnd/boulder_chain.c | MED |
| 0x80101fe0 (CASNOCTY) | FUN_80101fe0 | `CC_LoopSfxTick`      | src/physics/casnocty/loop_sfx.c | MED |
| 0x80100e78 (OILFIELD) | FUN_80100e78 | `OF_DebrisThrow`      | src/physics/oilfield/debris_throw.c | MED |
| 0x8010100c (SCRTBASE) | FUN_8010100c | `SB_DroneTick`        | src/physics/scrtbase/drone_tick.c | MED |
| 0x8010183c (SCRTBASE) | FUN_8010183c | `SB_TurretSpawn`      | src/physics/scrtbase/turret_spawn.c | MED |
| 0x80100ca0 (SCRTBASE) | FUN_80100ca0 | `SB_TerrainDeform_FromCrater` | src/physics/scrtbase/terrain_deform.c | HIGH-MED (heightmap deformation -- the "blow holes in the ground" effect) |
| 0x801011b0 (HOOVRDAM) | FUN_801011b0 | `HD_WaterBob`         | src/physics/hoovrdam/water.c   | MED |
| 0x801004e8 (CASNOCTY) | FUN_801004e8 | `CC_ManholeSpawn`     | src/physics/casnocty/manhole_spawn.c | MED |
| 0x801002bc (CASNOCTY) | FUN_801002bc | `CC_ManholeTick`      | src/physics/casnocty/manhole_tick.c | MED |
| 0x80101464 (HOOVRDAM) | FUN_80101464 | `HD_PowerlineTick`    | src/physics/hoovrdam/powerline.c | MED |
| 0x80101580 (HOOVRDAM) | FUN_80101580 | `HD_LeverTick`        | src/physics/hoovrdam/lever.c   | MED |
| 0x8010072c (WILDWEST) | FUN_8010072c | `WW_TrainTick`        | src/physics/wildwest/train_tick.c | MED |
| 0x8010076c (SCRTBASE) | FUN_8010076c | `SB_MissileChain`     | src/physics/scrtbase/missile_chain.c | MED |
| 0x80101028 (OILFIELD) | FUN_80101028 | `OF_RigDestroy`       | src/physics/oilfield/rig_destroy.c | MED |
| 0x80101efc (SKIRESRT) | FUN_80101efc | `SR_SnowEmit`         | src/physics/skiresrt/snow_emit.c | MED |
| 0x80100688 (WILDWEST) | FUN_80100688 | `WW_TrainInitOnSpawn` | src/physics/wildwest/train_init.c | MED |
| 0x801011a0 (LOAD)  | FUN_801011a0 | `Light_VertexColor`   | src/physics/light_eval.c       | HIGH-MED |
| 0x80100d1c (LOAD)  | FUN_80100d1c | `XOBF_LoadHealth`     | src/assets/xobf_health.c       | HIGH (confirms Vehicle/Object +0xc=health, +0xe=maxHealth) |
| 0x80100fa8 (LOAD)  | FUN_80100fa8 | `Sphere_TestWithBacktrack` | src/physics/sphere_test.c     | MED |
| 0x80101574 (LOAD)  | FUN_80101574 | `Hierarchy_TransformChain` | src/physics/hierarchy_transform.c | HIGH-MED |
| 0x80100e70 (VALLYFRM) | FUN_80100e70 | `VF_DetachAndStopSfx` | src/physics/vallyfrm/misc.c    | HIGH |
| 0x80100854 (SANDFACT) | FUN_80100854 | `SF_ElevatorFullTick` | src/physics/sandfact/elevator_full.c | MED |
| 0x801003c4 (HOOVRDAM) | FUN_801003c4 | `HD_TransformerAim`   | src/physics/hoovrdam/transformer.c | MED |
| 0x8010c690 (SHELL) | FUN_8010c690 | `CharSelect_BitMath`  | src/gameplay/char_select_bits.c | LOW |
| 0x80107668 + 9 more (SHELL) | (empty stubs) | `ShellStub_*` | src/gameplay/shell_stubs.c | HIGH (10 empty event-table slots collapsed) |
| 0x80107510, 0x80111464 (SHELL); 0x801060f8, 0x8010613c (LOAD) | (empty stubs) | `ShellStub_* / LoadStub_*` | src/gameplay/shell_stubs.c | HIGH (4 more empty event-table slots collapsed) |
| 0x8010a614 (SHELL) | FUN_8010a614 | `Shell_AllocLookupTables` | src/gameplay/shell_lookup.c | HIGH |
| 0x8010a6c4 (SHELL) | FUN_8010a6c4 | `Shell_FreeLookupTables`  | src/gameplay/shell_lookup.c | HIGH |
| 0x80106234 (LOAD)  | FUN_80106234 | `LoadDLL_ResetCallback` | src/gameplay/load_misc.c    | HIGH |
| 0x801062f4 (LOAD)  | FUN_801062f4 | `LoadDLL_CopyMDECParams`| src/gameplay/load_misc.c    | MED  |
| 0x80106398 (LOAD)  | FUN_80106398 | `LoadDLL_BitFlag_GP0_Hack`| src/gameplay/load_misc.c  | MED  |
| 0x80100fa4 (WILDWEST) | FUN_80100fa4 | `WW_GallowTick`       | src/physics/wildwest/gallow.c  | MED |
| 0x80101acc (SCRTBASE) | FUN_80101acc | `SB_FenceHit`         | src/physics/scrtbase/fence_hit.c | MED |
| 0x801002b0 (OILFIELD) | FUN_801002b0 | `OF_RigEmit`          | src/physics/oilfield/rig_emit.c | MED |

## Pass 3 Backlog -- CLOSED in pass 2

All entries previously listed here are now hand-cleaned. See:

| Original backlog                | Resolved file                                |
|---------------------------------|----------------------------------------------|
| AG_CraneMainUpdate (0x80100228) | src/physics/airgrave/tracker_dish.c          |
| AG_CraneSubState (0x80100c70)   | src/physics/airgrave/cruise_missile.c        |
| CL_BoulderRollPhysics (0x80100244) | src/physics/canynlnd/boulder_roll.c       |
| WW_TrainStateMachine (0x801009a8) | src/physics/wildwest/dynamite_keg.c        |
| WW_GallowDrop (0x8010178c)      | src/physics/wildwest/saloon_destruct.c       |
| WW_TrainSignalLogic (0x8010129c)| src/physics/wildwest/stage_coach.c           |
| HD_PaletteAnimator (0x80100c30) | src/physics/hoovrdam/siren_strobe.c          |
| HD_TransformerCascade (0x80101734) | src/physics/hoovrdam/spillway_grab.c      |
| SB_LevelMainUpdate (0x80100200) | src/physics/scrtbase/radar_sweep.c           |
| SB_RadarHit (0x80101904)        | src/physics/scrtbase/security_door.c         |
| SB_TurretAnimate (0x80101c58)   | src/physics/scrtbase/bunker_door.c           |
| V8_MainLoop (0x80013cac)        | src/gameplay/main_loop.c                     |
| Pad_Tick (0x800120d4)           | src/skipped/pad_input.c (controls scope)     |
| Menu_Pause (0x80012a90)         | src/gameplay/pause_menu.c                    |
| InsertOriginalCd (0x8001356c)   | src/gameplay/insert_original_cd.c            |
| MatchScore_AppendLine (0x800136c4) | src/gameplay/match_score.c                |
| ResultScreen_Build (0x8001392c) | src/gameplay/result_screen.c                 |
| Camera_BuildMatrix (0x8003e2fc) | src/physics/camera_build.c                   |

SHELL.DLL and LOAD.DLL (MDEC) renderer/video dispatchers remain
intentionally OUT-OF-SCOPE per CLAUDE.md (renderer / video are to be
rewritten). Seam contracts documented in `src/skipped/renderer.md`.

## Pass 2 batch additions (per-level destructibles)

| Address (DLL)           | Ghidra        | Final name                | File                                                | Confidence |
|-------------------------|---------------|---------------------------|-----------------------------------------------------|------------|
| 0x80100668 (OILFIELD)   | FUN_80100668  | `OF_SteamRig`             | src/physics/oilfield/steam_rig.c                    | MED        |
| 0x80100a30 (OILFIELD)   | FUN_80100a30  | `OF_BarrelRoll`           | src/physics/oilfield/barrel_roll.c                  | MED        |
| 0x80100e54 (SANDFACT)   | FUN_80100e54  | `SF_ConveyorGrab`         | src/physics/sandfact/conveyor_grab.c                | MED        |
| 0x801006d4 (CASNOCTY)   | FUN_801006d4  | `CC_BlimpPilot`           | src/physics/casnocty/blimp_pilot.c                  | MED        |
| 0x80100c88 (CASNOCTY)   | FUN_80100c88  | `CC_BlimpMain`            | src/physics/casnocty/blimp_main.c                   | MED        |
| 0x80101d00 (CASNOCTY)   | FUN_80101d00  | `CC_BombList`             | src/physics/casnocty/bomb_list.c                    | MED        |
| 0x80101464 (SKIRESRT)   | FUN_80101464  | `SK_LiftChairGrab`        | src/physics/skiresrt/lift_chair_grab.c              | MED        |
| 0x801005e0 (SKIRESRT)   | FUN_801005e0  | `SK_GondolaPosition`      | src/physics/skiresrt/gondola_position.c             | MED        |
| 0x80101a94 (SKIRESRT)   | FUN_80101a94  | `SK_IceChunk`             | src/physics/skiresrt/ice_chunk.c                    | MED        |
| 0x80101464 (SKIRESRT)   | FUN_80100974  | `SK_PowderBoulder`        | src/physics/skiresrt/powder_boulder.c               | MED        |
| 0x801016ac (SKIRESRT)   | FUN_801016ac  | `SK_SnowPulse`            | src/physics/skiresrt/snow_pulse.c                   | MED        |
| 0x80100964 (VALLYFRM)   | FUN_80100964  | `VF_SiloRotate`           | src/physics/vallyfrm/silo_rotate.c                  | MED        |
| 0x80100eb4 (VALLYFRM)   | FUN_80100eb4  | `VF_SiloSlide`            | src/physics/vallyfrm/silo_slide.c                   | MED        |
| 0x8010031c (VALLYFRM)   | FUN_8010031c  | `VF_WindmillAlarm`        | src/physics/vallyfrm/windmill_alarm.c               | MED        |
| 0x80100970 (SCRTBASE)   | FUN_80100970  | `SB_TurretTrack`          | src/physics/scrtbase/turret_track.c                 | MED        |
| 0x801010f4 (SCRTBASE)   | FUN_801010f4  | `SB_HomingMissile`        | src/physics/scrtbase/missile_track.c                | MED        |
| 0x8010266c (SCRTBASE)   | FUN_8010266c  | `SB_SiloDoor`             | src/physics/scrtbase/silo_door.c                    | MED        |
| 0x801010a8 (WILDWEST)   | FUN_801010a8  | `WW_BridgeCollapse`       | src/physics/wildwest/bridge_collapse.c              | MED        |
| 0x8010129c (WILDWEST)   | FUN_8010129c  | `WW_StageCoach`           | src/physics/wildwest/stage_coach.c                  | MED        |
| 0x801009a8 (WILDWEST)   | FUN_801009a8  | `WW_DynamiteKeg`          | src/physics/wildwest/dynamite_keg.c                 | MED        |
| 0x8010178c (WILDWEST)   | FUN_8010178c  | `WW_SaloonDestruct`       | src/physics/wildwest/saloon_destruct.c              | MED        |

Plus closures already listed in the "Pass 3 Backlog -- CLOSED" table
above: tracker_dish, cruise_missile, boulder_roll, blimp_*, siren_-
strobe, spillway_grab, radar_sweep, security_door, bunker_door,
turret_track, main_loop, pause_menu, insert_original_cd, match_score,
result_screen, camera_build.

Total Pass 2 batch additions: **39** new hand-cleaned files
(promoted from auto-stubs to MED-confidence). Total rename_log
entries now span roughly **403** addresses across the main EXE
and 12 overlays.
