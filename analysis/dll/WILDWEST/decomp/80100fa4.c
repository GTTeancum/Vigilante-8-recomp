// addr: 0x80100fa4  name: FUN_80100fa4

undefined4 FUN_80100fa4(uint *param_1,undefined4 param_2,uint *param_3)

{
  int iVar1;
  
  switch(param_2) {
  case 0:
    if (param_1[0xc] == 0) {
      FUN_800205f8/*0x800205f8*/(param_1);
    }
  case 3:
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
switchD_80100fd4_caseD_8:
    iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar1 != 0) {
      param_3 = param_1;
      FUN_8002002c/*0x8002002c*/(0x80065a18,9);
switchD_80100fd4_caseD_2:
      FUN_8003fc50/*0x8003fc50*/(param_1);
      goto switchD_80100fd4_caseD_9;
    }
    break;
  case 2:
    goto switchD_80100fd4_caseD_2;
  default:
    break;
  case 8:
    goto switchD_80100fd4_caseD_8;
  case 9:
switchD_80100fd4_caseD_9:
    if (*(short *)((int)param_3 + 6) != *(short *)((int)param_1 + 6)) {
      return 0;
    }
    *param_1 = *param_1 | 0x10000;
  case 6:
  case 1:
  }
  return 0;
}

