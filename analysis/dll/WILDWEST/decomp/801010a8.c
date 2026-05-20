// addr: 0x801010a8  name: FUN_801010a8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801010a8(uint *param_1,undefined4 param_2,int *param_3)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  
  piVar3 = param_3;
  switch(param_2) {
  case 0:
    if (param_1[0xc] == 0) {
      FUN_800205f8/*0x800205f8*/(param_1);
      piVar3 = param_3;
    }
  case 3:
    param_3 = piVar3;
    if (*(char *)(*piVar3 + 4) == '\a') {
      if ((param_1[0xe] != 0) && (((ushort)param_1[3] & 0x7f) < *(ushort *)(*piVar3 + 0xc))) {
        iVar1 = FUN_8003ff28/*0x8003ff28*/();
        *(undefined2 *)(iVar1 + 0xc) = 0;
        *(undefined4 *)(iVar1 + 0x88) = 0;
        *(undefined4 *)(iVar1 + 0x8c) = 0xfffff415;
        *(undefined4 *)(iVar1 + 0x90) = 0;
        uVar2 = FUN_8004410c/*0x8004410c*/();
        param_3 = (int *)&DAT_00000029;
        FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x29,iVar1 + 0x24);
        *(undefined4 *)(iVar1 + 100) = 0x8003cb64;
      }
      FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(*piVar3 + 0xc));
    }
    if (((*param_1 & 1) == 0) && ((uint *)param_3[3] != param_1)) {
      iVar1 = FUN_8003ff28/*0x8003ff28*/(param_1[0xe]);
      *(undefined4 *)(iVar1 + 100) = 0x8003cb64;
      *(undefined2 *)(iVar1 + 0xc) = 0;
      FUN_80020890/*0x80020890*/(param_1,300);
switchD_801010e0_caseD_8:
      FUN_80022320/*0x80022320*/(param_1,param_3);
switchD_801010e0_caseD_2:
      if (param_1[0x29] == 0) {
        FUN_8003fc50/*0x8003fc50*/(param_1);
switchD_801010e0_caseD_9:
        if (*(short *)((int)param_3 + 6) == *(short *)((int)param_1 + 6)) {
          *param_1 = *param_1 | 0x10000;
switchD_801010e0_caseD_6:
          goto switchD_801010e0_caseD_1;
        }
      }
    }
    break;
  case 1:
switchD_801010e0_caseD_1:
  default:
    break;
  case 2:
    goto switchD_801010e0_caseD_2;
  case 6:
    goto switchD_801010e0_caseD_6;
  case 8:
    goto switchD_801010e0_caseD_8;
  case 9:
    goto switchD_801010e0_caseD_9;
  }
  return 0;
}

