// addr: 0x801016ac  name: FUN_801016ac

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801016ac(uint *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  undefined4 uVar2;
  uint *puVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  undefined1 auStack_40 [4];
  short local_3c;
  short local_36;
  short local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  undefined2 local_20;
  undefined2 local_1e;
  undefined2 local_1c;
  
  switch(param_2) {
  case 0:
    cVar1 = (char)param_1[2] + -1;
    *(char *)(param_1 + 2) = cVar1;
    if (cVar1 == -1) {
      uVar7 = param_1[0xe];
      iVar4 = FUN_80017160/*0x80017160*/();
      iVar8 = ((iVar4 << 8) >> 0xf) + 0x1000;
      uVar2 = FUN_8001b038/*0x8001b038*/(uVar7,0x8000);
      uVar2 = FUN_8001b07c/*0x8001b07c*/(auStack_40,uVar2);
      CompMatrixLV/*0x8004cf04*/(uVar7 + 0x10,uVar2,auStack_40);
      puVar3 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x20,0xa0,8);
      *puVar3 = *puVar3 | 0x410;
      iVar4 = local_3c * iVar8;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      puVar3[0x22] = iVar4 >> 0xc;
      iVar4 = local_36 * iVar8;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      puVar3[0x23] = iVar4 >> 0xc;
      iVar8 = local_30 * iVar8;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      puVar3[0x24] = iVar8 >> 0xc;
      local_20 = 0;
      local_1e = 0;
      local_1c = FUN_80017160/*0x80017160*/();
      RotMatrixYXZ_gte/*0x8004dab4*/(&local_20,puVar3 + 4);
      puVar3[9] = local_2c;
      puVar3[10] = local_28;
      puVar3[0xb] = local_24;
      puVar3[0x19] = 0x8004042c;
      FUN_8001d4f0/*0x8001d4f0*/(param_1);
      *(undefined1 *)(param_1 + 2) = 8;
    }
    for (iVar4 = *(int *)(param_1[0xe] + 0x34); iVar4 != 0; iVar4 = *(int *)(iVar4 + 0x34)) {
      iVar6 = *(int *)(iVar4 + 0x88);
      *(int *)(iVar4 + 0x24) = *(int *)(iVar4 + 0x24) + iVar6;
      *(int *)(iVar4 + 0x28) = *(int *)(iVar4 + 0x28) + *(int *)(iVar4 + 0x8c);
      *(int *)(iVar4 + 0x2c) = *(int *)(iVar4 + 0x2c) + *(int *)(iVar4 + 0x90);
      iVar8 = iVar6;
      if (iVar6 < 0) {
        iVar8 = iVar6 + 0x3f;
      }
      iVar5 = *(int *)(iVar4 + 0x90);
      *(int *)(iVar4 + 0x88) = iVar6 - (iVar8 >> 6);
      iVar8 = iVar5;
      if (iVar5 < 0) {
        iVar8 = iVar5 + 0x3f;
      }
      *(int *)(iVar4 + 0x90) = iVar5 - (iVar8 >> 6);
      *(int *)(iVar4 + 0x8c) = *(int *)(iVar4 + 0x8c) + 0x38;
    }
    if (param_3 == 0) {
      return 0;
    }
    uVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar2);
  case 3:
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
switchD_801016f0_caseD_8:
    iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar4 != 0) {
      FUN_80020778/*0x80020778*/(param_1);
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
switchD_801016f0_caseD_1:
      *(byte *)(param_1 + 2) = *(byte *)((int)param_1 + 9) & 7;
      *param_1 = *param_1 | 0x80;
      uVar7 = 0x40000;
      if (0x40000 < (int)param_1[0x15]) {
        uVar7 = param_1[0x15];
      }
      param_1[0x15] = uVar7;
      cVar1 = FUN_8004410c/*0x8004410c*/();
      *(char *)((int)param_1 + 5) = cVar1;
      FUN_80044484/*0x80044484*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
      goto switchD_801016f0_caseD_4;
    }
    break;
  case 1:
    goto switchD_801016f0_caseD_1;
  default:
    break;
  case 4:
switchD_801016f0_caseD_4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    break;
  case 8:
    goto switchD_801016f0_caseD_8;
  }
  return 0;
}

