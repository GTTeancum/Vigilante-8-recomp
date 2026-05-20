// addr: 0x8010a86c  name: FUN_8010a86c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010a86c(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  short *psVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  short *psVar12;
  int iVar13;
  undefined1 local_310 [64];
  undefined4 local_2d0 [128];
  undefined4 local_d0 [36];
  undefined1 auStack_40 [8];
  undefined4 *local_38;
  undefined4 *local_30;
  
  uVar6 = 0x80110000;
  psVar12 = (short *)(DAT_801133a4 + (param_1 + param_2) * 0x200);
  iVar13 = DAT_801133a0 + (param_1 + param_2) * 0x28;
  iVar9 = 0;
  if (*psVar12 != 0x4353) {
    uVar10 = 0;
    do {
      iVar2 = thunk_FUN_800523a0/*0x80052384*/(DAT_801133a0 + (param_1 + iVar9) * 0x28,s_BASLUS_00510_80100d1c,0xc);
      uVar11 = uVar10;
      if ((iVar2 == 0) && (uVar11 = 1, 1 < uVar10)) {
        uVar11 = uVar10;
      }
      iVar9 = iVar9 + 1;
      uVar10 = uVar11;
    } while (iVar9 < 0xf);
    sprintf/*0x80053004*/(local_310,s_Vigilante_8__game___i_80100d2c,uVar11 + 1);
    sprintf/*0x80053004*/(iVar13,s_BASLUS_00510__i_80100d44,uVar11);
    V8_MemSet/*0x80044efc*/(psVar12 + 2,0,0x40);
    psVar12[2] = (short)local_310 * 0x100 | (ushort)((uint)local_310 >> 8) & 0xff;
    uVar6 = 1;
    *(undefined4 *)(iVar13 + 0x18) = 0x2000;
  }
  *(undefined1 *)(psVar12 + 1) = 0x11;
  piVar1 = DAT_80113394;
  *psVar12 = 0x4353;
  FUN_800185cc/*0x800185cc*/(auStack_40,*(undefined4 *)(*(int *)(*piVar1 + 0x14) + 0xb4),uVar6);
  uVar6 = local_38[1];
  uVar7 = local_38[2];
  uVar8 = local_38[3];
  *(undefined4 *)(psVar12 + 0x30) = *local_38;
  *(undefined4 *)(psVar12 + 0x32) = uVar6;
  *(undefined4 *)(psVar12 + 0x34) = uVar7;
  *(undefined4 *)(psVar12 + 0x36) = uVar8;
  uVar6 = local_38[5];
  uVar7 = local_38[6];
  uVar8 = local_38[7];
  *(undefined4 *)(psVar12 + 0x38) = local_38[4];
  *(undefined4 *)(psVar12 + 0x3a) = uVar6;
  *(undefined4 *)(psVar12 + 0x3c) = uVar7;
  *(undefined4 *)(psVar12 + 0x3e) = uVar8;
  psVar4 = psVar12 + 0x40;
  puVar3 = local_30 + 0x20;
  do {
    uVar6 = local_30[1];
    uVar7 = local_30[2];
    uVar8 = local_30[3];
    *(undefined4 *)psVar4 = *local_30;
    *(undefined4 *)(psVar4 + 2) = uVar6;
    *(undefined4 *)(psVar4 + 4) = uVar7;
    *(undefined4 *)(psVar4 + 6) = uVar8;
    local_30 = local_30 + 4;
    psVar4 = psVar4 + 8;
  } while (local_30 != puVar3);
  puVar3 = local_2d0;
  psVar4 = psVar12 + 0x100;
  do {
    uVar6 = *(undefined4 *)(psVar12 + 2);
    uVar7 = *(undefined4 *)(psVar12 + 4);
    uVar8 = *(undefined4 *)(psVar12 + 6);
    *puVar3 = *(undefined4 *)psVar12;
    puVar3[1] = uVar6;
    puVar3[2] = uVar7;
    puVar3[3] = uVar8;
    psVar12 = psVar12 + 8;
    puVar3 = puVar3 + 4;
  } while (psVar12 != psVar4);
  puVar5 = local_d0;
  puVar3 = (undefined4 *)&DAT_80056774;
  do {
    uVar6 = puVar3[1];
    uVar7 = puVar3[2];
    uVar8 = puVar3[3];
    *puVar5 = *puVar3;
    puVar5[1] = uVar6;
    puVar5[2] = uVar7;
    puVar5[3] = uVar8;
    puVar3 = puVar3 + 4;
    puVar5 = puVar5 + 4;
  } while (puVar3 != (undefined4 *)&DAT_800567d4);
  return;
}

