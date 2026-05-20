# PSY-Q Library Function Recognition

Common PSY-Q (Sony PSX SDK) functions to identify during classification.
Once spotted, they keep their original names (HIGH confidence) and are
tagged as `runtime` subsystem -- not in-scope for decomp, but their
presence anchors callers' purposes.

## Memory / Heap
- `InitHeap`, `malloc3`, `free3`, `realloc3`, `memset`, `memcpy`
- `bzero`, `bcopy`, `bcmp`

## libc-ish
- `printf`, `sprintf`, `strcmp`, `strcpy`, `strlen`, `strcat`, `atoi`

## CD-ROM
- `CdInit`, `CdReset`, `CdStatus`, `CdRead`, `CdReadSync`, `CdControl`,
  `CdControlF`, `CdControlB`, `CdSearchFile`, `CdPosToInt`, `CdIntToPos`,
  `CdReadyCallback`, `CdReadCallback`, `CdMix`, `DsAvail`

## GPU
- `ResetGraph`, `SetGraphDebug`, `SetVideoMode`, `SetDispMask`,
  `GsInitGraph`, `GsDefDispBuff`, `GsSetWorkBase`, `GsSetProjection`,
  `PutDispEnv`, `PutDrawEnv`, `ClearImage`, `LoadImage`, `StoreImage`,
  `DrawSync`, `DrawOTag`, `ClearOTag`, `ClearOTagR`, `AddPrim`,
  `OpenTIM`, `ReadTIM`, `CloseTIM`

## SPU / Audio
- `SpuInit`, `SpuStart`, `SpuSetTransferMode`, `SpuSetTransferStartAddr`,
  `SpuWrite`, `SpuMalloc`, `SpuSetCommonAttr`, `SpuSetVoiceAttr`,
  `SpuSetKey`, `SpuSetKeyOnWithAttr`

## Controller / Pad
- `PadInit`, `PadStart`, `PadStop`, `PadRead`,
  `InitPAD`, `StartPAD`, `StopPAD`

## Timing / IRQ
- `VSync`, `VSyncCallback`, `ResetCallback`, `OpenEvent`, `CloseEvent`,
  `EnableEvent`, `DisableEvent`, `WaitEvent`, `TestEvent`, `DeliverEvent`

## GTE / Math
- `InitGeom`, `SetGeomScreen`, `SetGeomOffset`, `RotMatrix`, `TransMatrix`,
  `ApplyMatrix`, `ApplyMatrixLV`, `MulMatrix`, `MulMatrix0`,
  `RotTrans`, `RotTransPers`, `RotMatrixX`, `RotMatrixY`, `RotMatrixZ`,
  `rsin`, `rcos`, `ratan2`, `SquareRoot0`

## File / Stream
- `open`, `close`, `read`, `write`, `lseek`

Detection cues:
- A leaf function that ends with a single `cop2` instruction (e.g. RTPS opcode 0x4A100001) -> wrapper for that GTE op.
- A small function calling `CdControl` then `CdReadSync` -> almost certainly `CdRead`.
- Functions referenced by entry-point prolog (jump table at start of EXE) -> usually PSY-Q startup.
- String "Sony Computer Entertainment" near function -> PSY-Q runtime init.
