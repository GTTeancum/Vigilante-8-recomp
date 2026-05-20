// addr: 0x80101464  name: FUN_80101464

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80101464(int param_1,undefined4 param_2,undefined4 *param_3)

{
  uint uVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint *puVar5;
  uint uVar6;
  
  puVar5 = (uint *)*param_3;
  if ((((*(short *)(param_3[3] + 6) == 0) && ((char)puVar5[1] == '\x02')) &&
      (*(short *)((int)puVar5 + 6) < 0)) && (*(short *)(_DAT_800659fc + 0xa2) != 0)) {
    uVar6 = puVar5[0x38];
    uVar1 = (uint)(*(short *)(param_1 + 0x42) == 0);
    if (*(short *)(_DAT_800659fc + 0xa0) != 0) {
      uVar1 = uVar1 ^ 1;
    }
    uVar1 = *(uint *)(_DAT_800659fc + uVar1 * 4 + 0x98);
    *(uint **)(uVar1 + 0x80) = puVar5;
    puVar5[0x19] = (uint)FUN_80101050;
    puVar5[0x1e] = uVar1;
    *puVar5 = *puVar5 & 0xfffffff7 | 0x1000020;
    puVar5[0x20] = (*(int *)(uVar1 + 0x24) - puVar5[9]) * 4;
    puVar5[0x21] = (*(int *)(uVar1 + 0x28) - (puVar5[10] - 0x10000)) * 4;
    puVar5[0x22] = (*(int *)(uVar1 + 0x2c) - puVar5[0xb]) * 4;
    FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar5 + 5),0);
    FUN_80020890/*0x80020890*/(puVar5,0x20);
    if (uVar6 != 0) {
      puVar2 = (undefined4 *)Heap_AllocOrRetry/*0x800116f4*/(0x30);
      uVar3 = *(undefined4 *)(uVar6 + 0x4c);
      uVar4 = *(undefined4 *)(uVar6 + 0x50);
      *puVar2 = *(undefined4 *)(uVar6 + 0x48);
      puVar2[1] = uVar3;
      puVar2[2] = uVar4;
      puVar2[3] = 0x78;
      puVar2[4] = *(undefined4 *)(uVar1 + 0x48);
      puVar2[5] = *(int *)(uVar1 + 0x4c) + -0x32000;
      puVar2[6] = *(int *)(uVar1 + 0x50) + -0x64000;
      puVar2[7] = 0;
      func_0x8003dbb0(uVar6);
    }
    uVar3 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),5,puVar5 + 9);
  }
  func_0x800223dc(param_1,param_2,param_3);
  return;
}

