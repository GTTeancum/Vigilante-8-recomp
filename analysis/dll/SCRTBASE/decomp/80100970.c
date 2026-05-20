// addr: 0x80100970  name: FUN_80100970

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100970(int param_1,undefined4 param_2,int *param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  undefined2 uVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int *piVar10;
  int unaff_s2;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  
  switch(param_2) {
  case 0:
    iVar8 = *(int *)(param_1 + 0x38);
    unaff_s2 = *(int *)(param_1 + 0x78);
    uVar3 = FUN_8001d624/*0x8001d624*/(iVar8);
    FUN_800435c0/*0x800435c0*/(uVar3,unaff_s2 + 0x48,&local_30);
    iVar4 = ratan2/*0x8004ecd4*/(local_30,local_28);
    iVar7 = (iVar4 << 0x14) >> 0x14;
    iVar4 = ratan2/*0x8004ecd4*/(local_2c,local_28);
    iVar4 = iVar4 * -0x100000 >> 0x14;
    if (iVar7 < 0) {
      iVar7 = iVar7 + 3;
    }
    *(short *)(iVar8 + 0x42) = *(short *)(iVar8 + 0x42) + (short)(iVar7 >> 2);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 3;
    }
    iVar7 = (int)*(short *)(iVar8 + 0x40) + (iVar4 >> 2);
    iVar4 = -0x155;
    if ((iVar7 < -0x155) || (uVar5 = 0x155, iVar4 = iVar7, iVar7 < 0x156)) {
      uVar5 = (undefined2)iVar4;
    }
    *(undefined2 *)(iVar8 + 0x40) = uVar5;
    FUN_8001d708/*0x8001d708*/();
    if ((local_28 < 0xfa001) && (*(short *)(unaff_s2 + 0xc) != 0)) {
      return 0;
    }
    *(undefined4 *)(param_1 + 0x78) = 0;
    break;
  case 1:
    goto switchD_801009bc_caseD_1;
  case 2:
    break;
  case 3:
    goto switchD_801009bc_caseD_3;
  default:
    goto switchD_801009bc_caseD_4;
  case 8:
    goto switchD_801009bc_caseD_8;
  }
  if (*(int *)(param_1 + 0x78) == 0) goto LAB_80100ba0;
  puVar1 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x58),0x1e3,0x80,8);
  puVar2 = (undefined4 *)FUN_80031300/*0x80031300*/(param_1,*(undefined4 *)(param_1 + 0x38),0x1e1,0x98,puVar1);
  *puVar2 = 0x84;
  *(undefined2 *)(puVar2 + 3) = 0x32;
  puVar2[0x19] = FUN_8010076c;
  *(undefined2 *)((int)puVar2 + 0x96) = 4;
  *(undefined2 *)(puVar2 + 0x25) = 8;
  FUN_800202f4/*0x800202f4*/(puVar2);
  *puVar1 = 0x10;
  puVar1[0x19] = 0x8003e80c;
  FUN_800207c4/*0x800207c4*/();
  uVar3 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),0,puVar2 + 9);
  while( true ) {
    *(undefined4 **)(param_1 + 0x78) = puVar2;
    FUN_80020744/*0x80020744*/(param_1);
    uVar3 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),2,unaff_s2);
LAB_80100ba0:
    if ((int *)*_DAT_80065a18 == (int *)0x0) break;
    unaff_s2 = param_1 + 0x48;
    piVar6 = _DAT_80065a18;
    piVar9 = (int *)*_DAT_80065a18;
    while (((puVar2 = (undefined4 *)piVar6[2], *(char *)(puVar2 + 1) != '\x02' ||
            (*(short *)(puVar2 + 3) == 0)) ||
           (iVar4 = FUN_80016aac/*0x80016aac*/(unaff_s2,puVar2 + 0x12), 0xc7fff < iVar4))) {
      piVar10 = (int *)*piVar9;
      piVar6 = piVar9;
      piVar9 = piVar10;
      if (piVar10 == (int *)0x0) goto LAB_80100c14;
    }
  }
LAB_80100c14:
  param_3 = (int *)&DAT_0000003c;
switchD_801009bc_caseD_3:
  if (*(char *)(*param_3 + 4) == '\a') {
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
switchD_801009bc_caseD_8:
    iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar4 != 0) {
      func_0x80020844(param_1);
      FUN_80020778/*0x80020778*/(param_1);
switchD_801009bc_caseD_1:
      FUN_80020890/*0x80020890*/(param_1,*(byte *)(param_1 + 9) + 0x1e);
    }
  }
switchD_801009bc_caseD_4:
  return 0;
}

