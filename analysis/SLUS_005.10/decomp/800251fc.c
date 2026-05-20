// addr: 0x800251fc  name: FUN_800251fc

void FUN_800251fc(undefined2 param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined *puVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int *piVar9;
  undefined *p1;
  
  iVar6 = 0;
  piVar9 = &DAT_8007a8a0;
  uRam00000814 = 0;
  uRam0000082c = param_1;
  do {
    if (*piVar9 != 0) {
      FUN_80045088();
    }
    iVar6 = iVar6 + 1;
    piVar9 = piVar9 + 1;
  } while (iVar6 < 0x40);
  iVar6 = FUN_800116f4(0x3000);
  iVar7 = 1;
  puVar2 = &DAT_8007a8a4;
  DAT_8007a8a0 = iVar6;
  do {
    *puVar2 = 0;
    iVar7 = iVar7 + 1;
    puVar2 = puVar2 + 1;
  } while (iVar7 < 0x40);
  iVar7 = 0;
  do {
    iVar4 = 0;
    iVar5 = iVar7 << 7;
    do {
      iVar1 = iVar4 + iVar7 * 0x40;
      iVar4 = iVar4 + 1;
      *(undefined2 *)(iVar6 + iVar5) = 0x45ff;
      *(undefined1 *)(iVar6 + iVar1 + 0x2000) = 0;
      iVar5 = iVar5 + 2;
    } while (iVar4 < 0x40);
    iVar7 = iVar7 + 1;
  } while (iVar7 < 0x40);
  iVar7 = -1;
  do {
    iVar5 = 0;
    iVar4 = iVar7 << 7;
    do {
      *(int *)((int)&DAT_800911a0 + iVar4) = iVar6;
      iVar5 = iVar5 + 1;
      iVar4 = iVar4 + 4;
    } while (iVar5 < 0x20);
    iVar7 = iVar7 + 1;
  } while (iVar7 < 0x21);
  uVar8 = 0;
  puVar3 = &UNK_80092220;
  do {
    puVar3[3] = 6;
    puVar3[7] = 0x30;
    uVar8 = uVar8 + 1;
    puVar3 = puVar3 + 0x1c;
  } while (uVar8 < 0x900);
  uVar8 = 0;
  puVar3 = &UNK_8007a9a0;
  do {
    puVar3[3] = 9;
    puVar3[7] = 0x34;
    puVar3[0x1f] = 0x34;
    puVar3[0x13] = 0x34;
    uVar8 = uVar8 + 1;
    puVar3 = puVar3 + 0x28;
  } while (uVar8 < 0x800);
  uVar8 = 0;
  puVar3 = &UNK_8008e9a0;
  do {
    puVar3[3] = 0xc;
    puVar3[7] = 0x3c;
    uVar8 = uVar8 + 1;
    puVar3 = puVar3 + 0x34;
  } while (uVar8 < 0x20);
  iVar6 = 0;
  puVar3 = &DAT_800910c0;
  p1 = &DAT_800910d8;
  do {
    puVar3[3] = 5;
    puVar3[7] = 0x28;
    p1[3] = 5;
    p1[7] = 0x28;
    MargePrim(puVar3,p1);
    p1 = p1 + 0x30;
    iVar6 = iVar6 + 1;
    puVar3 = puVar3 + 0x30;
  } while (iVar6 < 2);
  return;
}

