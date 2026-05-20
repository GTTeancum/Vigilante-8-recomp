// addr: 0x80102264  name: FUN_80102264

void FUN_80102264(void)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 *puVar4;
  uint local_18;
  int local_14;
  int local_10 [2];
  
  Stream_OpenByName/*0x800159b4*/(s_Shell_Vehicles_exp_80100444);
  Iff_ReadChunkData/*0x800225d4*/(&local_18,local_10);
  local_10[0] = local_14;
  if (local_14 != 0) {
    puVar4 = (undefined4 *)&DAT_800737a0;
    do {
      iVar1 = Iff_ReadChunkData/*0x800225d4*/(&local_18,local_10);
      uVar3 = 0x584f0000;
      if (iVar1 != 0) {
        Heap_Free/*0x80045088*/(iVar1);
      }
      if ((local_18 >> 0x18 | local_18 >> 8 & 0xff00 | (local_18 & 0xff00) << 8 | local_18 << 0x18)
          == (uVar3 | 0x4246)) {
        uVar2 = FUN_8002263c/*0x8002263c*/(local_14,0);
        *puVar4 = uVar2;
        puVar4 = puVar4 + 1;
      }
    } while (local_10[0] != 0);
  }
  Stream_Close/*0x80015a00*/();
  return;
}

