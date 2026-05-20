// addr: 0x801057f0  name: FUN_801057f0

void FUN_801057f0(ushort *param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  
  iVar8 = 0;
  piVar9 = (int *)&DAT_8007a8a0;
  do {
    if (*piVar9 == 0) break;
    iVar8 = iVar8 + 1;
    piVar9 = piVar9 + 1;
  } while (iVar8 < 0x40);
  uVar2 = Heap_AllocOrRetry/*0x800116f4*/(0x3000);
  *(undefined4 *)(&DAT_8007a8a0 + iVar8 * 4) = uVar2;
  iVar7 = 0;
  do {
    iVar5 = 0;
    iVar6 = iVar7 << 7;
    puVar4 = (undefined1 *)((int)param_1 + 3);
    do {
      *(ushort *)(*(int *)(&DAT_8007a8a0 + iVar8 * 4) + iVar6) =
           (*param_1 >> 8 | *param_1 << 8) - 0x200 | (ushort)((byte)puVar4[-1] >> 3) << 0xb;
      param_1 = param_1 + 2;
      uVar1 = *puVar4;
      iVar3 = iVar5 + iVar7 * 0x40;
      iVar5 = iVar5 + 1;
      puVar4 = puVar4 + 4;
      *(undefined1 *)(*(int *)(&DAT_8007a8a0 + iVar8 * 4) + iVar3 + 0x2000) = uVar1;
      iVar6 = iVar6 + 2;
    } while (iVar5 < 0x40);
    iVar7 = iVar7 + 1;
  } while (iVar7 < 0x40);
  return;
}

