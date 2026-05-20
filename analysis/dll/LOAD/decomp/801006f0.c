// addr: 0x801006f0  name: FUN_801006f0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint * FUN_801006f0(undefined4 param_1,int param_2)

{
  undefined1 uVar1;
  undefined1 uVar2;
  short sVar3;
  undefined2 uVar4;
  short sVar5;
  undefined2 uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined1 *puVar10;
  int iVar11;
  uint *puVar12;
  uint *puVar13;
  undefined4 *puVar14;
  int *piVar15;
  int *piVar16;
  int *piVar17;
  uint uVar18;
  undefined4 local_res0 [4];
  undefined1 auStack_92 [10];
  uint local_88;
  uint local_84;
  uint local_80;
  undefined4 local_78;
  undefined2 local_74;
  undefined1 auStack_70 [64];
  undefined4 local_30;
  
  local_res0[0] = param_1;
  local_30 = func_0x8002249c(local_res0);
  uVar7 = func_0x8002249c(local_res0);
  sVar3 = func_0x800224b4(local_res0);
  uVar8 = func_0x800224ec(local_res0);
  uVar18 = uVar8 & 0xfff867fe;
  local_88 = func_0x800224ec(local_res0);
  iVar9 = func_0x800224ec(local_res0);
  local_84 = iVar9 - 0x100000;
  local_80 = func_0x800224ec(local_res0);
  uVar4 = func_0x800224b4(local_res0);
  local_78 = CONCAT22(local_78._2_2_,uVar4);
  uVar4 = func_0x800224b4(local_res0);
  local_78 = CONCAT22(uVar4,(undefined2)local_78);
  local_74 = func_0x800224b4(local_res0);
  sVar5 = func_0x800224b4(local_res0);
  iVar9 = sVar5 + 0x12;
  uVar4 = func_0x800224b4(local_res0);
  uVar6 = func_0x800224ec(local_res0);
  V8_MemCopy/*0x80044c44*/(auStack_70,local_res0[0],param_2 + -0x22);
  auStack_70[param_2 + -0x22] = 0;
  puVar10 = (undefined1 *)func_0x80011aa8(0,auStack_70);
  if ((puVar10 == (undefined1 *)0x0) &&
     (puVar10 = (undefined1 *)func_0x80011a38(*(undefined4 *)(_DAT_80065a38 + 4),auStack_70),
     puVar10 == (undefined1 *)0x0)) {
    puVar10 = &SUB_800223dc;
  }
  if ((uVar8 & 4) != 0) {
    iVar11 = FUN_80017160/*0x80017160*/();
    _DAT_800659d0 =
         (undefined2)(iVar11 * **(int **)(*(int *)(&DAT_800737a0 + iVar9 * 4) + 4) >> 0xf);
  }
  if (6 < uVar7) {
    return (uint *)0x0;
  }
  puVar12 = (uint *)(&switchD_801008a4::switchdataD_80100008)[uVar7];
  uVar2 = (undefined1)uVar7;
  switch(uVar7) {
  case 0:
    puVar12 = (uint *)FUN_80021b80/*0x80021b80*/(puVar10,*(undefined4 *)(&DAT_800737a0 + iVar9 * 4),uVar4,
                                      (uVar8 & 4) << 1);
    *puVar12 = uVar18;
    *(undefined1 *)(puVar12 + 1) = uVar2;
    *(short *)((int)puVar12 + 6) = sVar3;
    *(undefined1 *)(puVar12 + 2) = (undefined1)local_30;
    puVar12[0x12] = local_88;
    puVar12[0x13] = local_84;
    puVar12[0x14] = local_80;
    puVar12[0x10] = local_78;
    *(undefined2 *)(puVar12 + 0x11) = local_74;
    *(undefined2 *)((int)puVar12 + 0xe) = uVar6;
    *(undefined2 *)(puVar12 + 3) = uVar6;
    uVar1 = FUN_80017160/*0x80017160*/();
    *(undefined1 *)((int)puVar12 + 9) = uVar1;
    puVar12[0x19] = (uint)puVar10;
    FUN_8001d708/*0x8001d708*/(puVar12);
    FUN_8001dc1c/*0x8001dc1c*/(puVar12);
    if ((code *)puVar12[0x19] != (code *)0x0) {
      (*(code *)puVar12[0x19])(puVar12,1,0);
    }
    FUN_8001b0c4/*0x8001b0c4*/(puVar12);
    if (((*puVar12 & 8) != 0) && (puVar12[0x1c] == 0)) {
      FUN_8003e730/*0x8003e730*/(puVar12);
    }
    if ((*puVar12 & 4) != 0) {
      FUN_8001fe50/*0x8001fe50*/(0x80065a80,puVar12);
    }
    if ((*puVar12 & 0x80) != 0) {
      FUN_8001fe50/*0x8001fe50*/(0x80065a60,puVar12);
    }
    FUN_8001ec48/*0x8001ec48*/(puVar12);
    FUN_8001fe50/*0x8001fe50*/(&DAT_80107da0,puVar12);
  default:
    puVar13 = (uint *)FUN_80021b80/*0x80021b80*/(puVar10,*(undefined4 *)(&DAT_800737a0 + iVar9 * 4),uVar4,
                                      (uVar8 & 4) << 1);
    *puVar13 = uVar18;
    *(undefined1 *)(puVar13 + 1) = uVar2;
    *(short *)((int)puVar13 + 6) = sVar3;
    *(undefined1 *)(puVar13 + 2) = (undefined1)local_30;
    puVar13[0x12] = local_88;
    puVar13[0x13] = local_84;
    puVar13[0x14] = local_80;
    puVar13[0x10] = local_78;
    *(undefined2 *)(puVar13 + 0x11) = local_74;
    uVar7 = puVar13[0xe];
    *(undefined2 *)((int)puVar13 + 0xe) = uVar6;
    *(undefined2 *)(puVar13 + 3) = uVar6;
    for (; uVar7 != 0; uVar7 = *(uint *)(uVar7 + 0x34)) {
      *(undefined2 *)(uVar7 + 0xe) = uVar6;
      *(undefined2 *)(uVar7 + 0xc) = uVar6;
    }
    uVar1 = FUN_80017160/*0x80017160*/();
    *(undefined1 *)((int)puVar13 + 9) = uVar1;
    puVar13[0x19] = (uint)puVar10;
    iVar11 = FUN_8002036c/*0x8002036c*/(puVar13);
    puVar12 = (uint *)0x0;
    if (iVar11 != 0) {
      FUN_8001b0c4/*0x8001b0c4*/(puVar13);
switchD_801008a4_caseD_5:
      puVar12 = (uint *)FUN_8001d470/*0x8001d470*/(0x80);
      puVar12[0x19] = (uint)puVar10;
      uVar7 = *(uint *)(&DAT_800737a0 + iVar9 * 4);
      *(undefined2 *)((int)puVar12 + 10) = uVar4;
      *puVar12 = uVar18;
      *(undefined1 *)(puVar12 + 1) = uVar2;
      *(short *)((int)puVar12 + 6) = sVar3;
      *(undefined1 *)(puVar12 + 2) = (undefined1)local_30;
      puVar12[0x16] = uVar7;
      puVar12[0x12] = local_88;
      puVar12[0x13] = local_84;
      puVar12[0x14] = local_80;
      puVar12[0x10] = local_78;
      *(undefined2 *)(puVar12 + 0x11) = local_74;
      *(undefined2 *)((int)puVar12 + 0xe) = uVar6;
      *(undefined2 *)(puVar12 + 3) = uVar6;
      uVar1 = FUN_80017160/*0x80017160*/();
      *(undefined1 *)((int)puVar12 + 9) = uVar1;
      FUN_8001d708/*0x8001d708*/(puVar12);
      piVar16 = (int *)*_DAT_80065a50;
      piVar17 = _DAT_80065a50;
      if ((int *)*_DAT_80065a50 != (int *)0x0) {
        do {
          piVar15 = piVar16;
          piVar16 = piVar15;
          if (sVar3 <= *(short *)(piVar17[2] + 6)) break;
          piVar16 = (int *)*piVar15;
          piVar17 = piVar15;
        } while (piVar16 != (int *)0x0);
        if ((piVar16 != (int *)0x0) && (uVar7 = piVar17[2], sVar3 == *(short *)(uVar7 + 6))) {
          iVar9 = *(int *)(uVar7 + 0x34);
          while (iVar9 != 0) {
            uVar7 = *(uint *)(uVar7 + 0x34);
            iVar9 = *(int *)(uVar7 + 0x34);
          }
          *(uint **)(uVar7 + 0x34) = puVar12;
          puVar12[0xf] = uVar7;
        }
      }
      puVar14 = (undefined4 *)FUN_80022c54/*0x80022c54*/(puVar12);
      piVar16 = (int *)piVar17[1];
      *piVar16 = (int)puVar14;
      piVar17[1] = (int)puVar14;
      *puVar14 = piVar17;
      puVar14[1] = piVar16;
      goto switchD_801008a4_caseD_6;
    }
    break;
  case 1:
    break;
  case 5:
    goto switchD_801008a4_caseD_5;
  case 6:
switchD_801008a4_caseD_6:
    puVar12 = (uint *)FUN_8001d470/*0x8001d470*/(0x94);
    *puVar12 = uVar18;
    *(undefined1 *)(puVar12 + 1) = uVar2;
    *(short *)((int)puVar12 + 6) = sVar3;
    *(undefined1 *)(puVar12 + 2) = (undefined1)local_30;
    puVar12[0x12] = local_88;
    puVar12[0x13] = local_84;
    puVar12[0x14] = local_80;
    puVar12[0x10] = local_78;
    *(undefined2 *)(puVar12 + 0x11) = local_74;
    uVar2 = FUN_80017160/*0x80017160*/();
    *(undefined1 *)((int)puVar12 + 9) = uVar2;
    FUN_8001d708/*0x8001d708*/(puVar12);
    FUN_8001fe50/*0x8001fe50*/(&DAT_80107d90,puVar12);
  }
  return puVar12;
}

