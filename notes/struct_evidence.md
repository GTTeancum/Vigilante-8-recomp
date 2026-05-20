# Struct Evidence (Pass 2 Source of Truth)

This document captures the cross-function offset evidence used to
populate `include/structs.h`. Each entry should cite at least two
independent call sites. When pass 2 promotes a LOW field to MED/HIGH,
add the new evidence here.

## Vehicle

Anchor globals: `puRam000007d0` (P1), `puRam000007d4` (P2).
Evidence sweep tool: `tools/triage.py` would walk every function whose
parameters receive the anchor via the call graph (see the `STRUCT_PASS`
section of `progress.log`).

| Off  | Type | Sites | Field          | Confidence | Evidence |
|------|------|-------|----------------|------------|----------|
| 0x00 | u32  | 6     | inputFlags     | HIGH       | Main loop ANDs with 0xfffffffd, 0x20000000, 0x1000000; bit 0x4000 in init |
| 0x0c | i16  | 6     | statusFlags    | HIGH       | `(short)puRam000007d0[3]` == 0 check + LSB picks alt-model parity |
| 0x0e | i16  | 2     | statusPad      | HIGH       | Adjacent to 0x0c, same access width |
| 0x54 | u32* | 2     | listLink       | MED        | 24-bit masked store; assigned during pool init |
| 0xb3 | u8   | 6     | damageBits     | MED        | Set after collision; consumed by HUD blip color |
| 0xbc | i16  | 4     | healthMaybe    | MED        | Compared against 0; decremented under collision events |
| 0xd0 | i8   | 2     | controlFlags   | LOW        | Inferred from per-tick read; no clear consumer yet |
| 0xe0 | void*| 4     | modelData      | HIGH       | Passed to Render_VehicleSetTrans (renderer) and to Camera_BuildMatrix |
| 0xe4 | i32  | 4     | drawCallback   | MED        | Stored into per-frame OT prims |
| 0xec | i32  | 8     | posX_q1715     | HIGH       | Fed into GTE_RotateLong (17.15 packed); 3-axis pattern |
| 0xf0 | i32  | 6     | posY_q1715     | HIGH       | Adjacent to 0xec, same access pattern |
| 0xf4 | i32  | 6     | posZ_q1715     | HIGH       | Adjacent to 0xf0 |
| 0xf8 | void*| 4     | altModelData   | HIGH       | `puRam000007d0[0x3e]` indexes 0xf8; selected by statusFlags & 2 |
| 0x120| i16  | 2     | angleMaybe     | MED        | Compared against 0x8a inside renderer model index; pair with 0x0c |

## MatchState

Composed of ~12 individual globals -- pass 2 should consolidate.

| VA       | Type | Field            | Confidence | Note |
|----------|------|------------------|------------|------|
| 0x80000015 | u8 | g_matchMode      | HIGH       | Enum: 0=shell, 1/2=play, 3=end, 5=demo |
| 0x800007d0 | Vehicle* | g_player1Vehicle | HIGH | NULL == no player |
| 0x800007d4 | Vehicle* | g_player2Vehicle | HIGH | NULL == single player |
| 0x80000010 | i32 | g_splitScreenMode| HIGH       | 0=1P, 1=2P horiz, 2=2P vert |
| 0x80000008 | i32 | g_drawBufIndex   | HIGH       | Toggles 0/1 each frame |
| 0x80000004 | i32 | g_dispBufIndex   | HIGH       | Toggles 0/1 each frame |
| 0x80000018 | i8  | g_dispOffsetX    | LOW        |  |
| 0x80000019 | i8  | g_dispOffsetY    | LOW        |  |
| 0x800006a4 | i32 | g_vsyncLatched   | HIGH       | Spin target |
| 0x800006a8 | i32 | g_vsyncCounter   | HIGH       | IRQ-written |
| 0x80065c30 | u32 | g_matchConfigFlags | LOW      |  |

## SndBank

Layout @ `Audio_ParseSND` (src/assets/snd_parse.c) -- HIGH:

```c
typedef struct {
    uint16_t nSamples;
    uint16_t spuBaseIn8b;     /* SPU address divided by 8 */
    uint32_t sampleOff[/*nSamples*/];  /* SPU offsets in 8-byte units */
} SndBank;
```

Companion: `Audio_FreeSND` (src/assets/snd_loader.c) frees
`SpuFree(spuBaseIn8b << 3)` and `Heap_Free(bank)`.

## Iso (DirRecord + FileRecord)

Already documented in src/assets/iso9660.c header comments; not
duplicated here.

## Pass 2 Vehicle field additions

| Off  | Type | Source                  | Confidence | Field             |
|------|------|-------------------------|------------|-------------------|
| 0xc  | u16  | Damage_AccumulateOrFire + XOBF_LoadHealth | HIGH | health |
| 0xe  | u16  | Damage_AccumulateOrFire + XOBF_LoadHealth | HIGH | maxHealth |
| 0xe4 | i32  | Vehicle_TryAcquireTarget | HIGH      | currentTarget |
| 0xba | u8   | MatchScore_AppendLine    | MED-HIGH  | weight |
| 0xbb | u8   | MatchScore_AppendLine    | MED-HIGH  | skill |
| 0xb3 | u8   | Vehicle_TryAcquireTarget + earlier evidence | HIGH | damageBits (table idx for +0x110) |
| 0xd0 | u8   | Vehicle_TryAcquireTarget | HIGH      | controlFlags (AI skill threshold) |
| 0x110+ | u32[?] | Vehicle_TryAcquireTarget | MED   | candidateTable (per-damageBits target type pointers) |

## QuestRecord (Quest.bin per-mission entry)

Layout @ `Quest_Load` (src/assets/quest_loader.c) -- HIGH:

```c
typedef struct {
    uint8_t   questId;       /* +0x00 -- mission selector / index */
    uint8_t   bannerKind;    /* +0x01 -- consumed by V8_MainLoop  */
    uint16_t  configA;       /* +0x02 -- -> uRam000006f0 (sky?)   */
    uint16_t  configB;       /* +0x04 -- -> uRam000007dc (cue?)   */
    uint16_t  reserved;      /* +0x06 */
    void     *fld2;          /* +0x08 -- relocated by Quest_Load  */
    char     *bannerText;    /* +0x0c -- relocated by Quest_Load  */
} QuestRecord;  /* exactly 16 bytes -- size confirmed by cRam00000600 * 0x10 */
```

Container (Quest.bin top-level):

```c
typedef struct {
    uint32_t nCharacters;
    struct {
        uint32_t nQuests;
        uint32_t recsRel;    /* relative file offset, rewritten by loader */
    } groups[/*nCharacters*/];
    /* ...per-character QuestRecord[] payloads ... */
} QuestBin;
```

Consumer wiring (V8_MainLoop in src/gameplay/main_loop.c:211):
```c
QuestRecord *q = (QuestRecord *)*(int *)(iRam00000608 +
                  DAT_80065674 * 8 + 8) + cRam00000600;
banner = q->bannerText;
```
