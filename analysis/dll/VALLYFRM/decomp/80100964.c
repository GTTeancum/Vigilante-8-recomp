// addr: 0x80100964  name: FUN_80100964

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100964(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  char cVar4;
  undefined2 uVar5;
  int iVar6;
  undefined4 *puVar7;
  uint uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int iVar13;
  uint uVar14;
  uint uVar15;
  int iVar16;
  
  iVar6 = param_1;
  if (param_2 == 1) goto LAB_80100bd8;
  if (param_2 == 0) {
LAB_801009b8:
    *(short *)(*(int *)(param_1 + 0x38) + 0x44) = *(short *)(*(int *)(param_1 + 0x38) + 0x44) + 0x44
    ;
    if (param_3 == 0) {
      return 0;
    }
    FUN_8001d708/*0x8001d708*/(*(undefined4 *)(param_1 + 0x38));
LAB_801009e8:
    iVar6 = FUN_8002239c/*0x8002239c*/(param_1,param_3);
    if (iVar6 == 0) {
      return 0;
    }
    iVar6 = 1;
  }
  else {
    if (param_2 == 3) goto LAB_801009e8;
    if (param_2 != 8) goto LAB_801009b8;
  }
  iVar6 = FUN_80022320/*0x80022320*/(iVar6,param_3);
  if (iVar6 == 0) {
    return 0;
  }
  iVar6 = 0;
  puVar7 = (undefined4 *)FUN_8001d624/*0x8001d624*/(*(undefined4 *)(param_1 + 0x38));
  iVar16 = 0;
  uVar15 = 0xffffffff;
  uVar14 = 0xffffffff;
  piVar3 = (int *)*_DAT_80065a18;
  piVar2 = _DAT_80065a18;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    iVar13 = piVar2[2];
    if ((*(char *)(iVar13 + 4) == '\x02') && (*(short *)(iVar13 + 0xc) != 0)) {
      uVar8 = FUN_80016aac/*0x80016aac*/(param_1 + 0x48,iVar13 + 0x48);
      if ((0 < *(short *)(iVar13 + 6)) && (uVar8 < uVar14)) {
        uVar8 = 1;
        uVar14 = uVar8;
        iVar16 = iVar13;
      }
      if (uVar8 < uVar15) {
        uVar15 = uVar8;
        iVar6 = iVar13;
      }
    }
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  if (iVar16 == 0) {
    iVar16 = iVar6;
  }
  uVar9 = FUN_8001d564/*0x8001d564*/(*(undefined4 *)(param_1 + 0x38));
  puVar10 = (undefined4 *)Heap_Realloc/*0x80045134*/(uVar9,0x98);
  puVar10[0x21] = iVar16;
  *(undefined2 *)((int)puVar10 + 6) = *(undefined2 *)(param_1 + 6);
  uVar9 = puVar7[1];
  uVar11 = puVar7[2];
  uVar12 = puVar7[3];
  puVar10[4] = *puVar7;
  puVar10[5] = uVar9;
  puVar10[6] = uVar11;
  puVar10[7] = uVar12;
  uVar9 = puVar7[5];
  uVar11 = puVar7[6];
  uVar12 = puVar7[7];
  puVar10[8] = puVar7[4];
  puVar10[9] = uVar9;
  puVar10[10] = uVar11;
  puVar10[0xb] = uVar12;
  uVar9 = puVar7[6];
  uVar11 = puVar7[7];
  puVar10[0x12] = puVar7[5];
  puVar10[0x13] = uVar9;
  puVar10[0x14] = uVar11;
  *(undefined1 *)(puVar10 + 1) = 7;
  *puVar10 = 0x80;
  *(undefined2 *)(puVar10 + 3) = 100;
  puVar10[0x19] = FUN_801005e8;
  FUN_800202f4/*0x800202f4*/(puVar10);
  puVar10[0x22] = (int)*(short *)(puVar10 + 5) << 1;
  puVar10[0x23] = (int)*(short *)((int)puVar10 + 0x1a) << 1;
  puVar10[0x24] = (int)*(short *)(puVar10 + 8) << 1;
  cVar4 = FUN_8004410c/*0x8004410c*/();
  *(char *)((int)puVar10 + 5) = cVar4;
  FUN_8004483c/*0x8004483c*/((int)cVar4,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),1,puVar10 + 9);
  iVar6 = 1;
LAB_80100bd8:
  uVar5 = FUN_80017160/*0x80017160*/(iVar6);
  *(undefined2 *)(*(int *)(param_1 + 0x38) + 0x44) = uVar5;
  return 0;
}

