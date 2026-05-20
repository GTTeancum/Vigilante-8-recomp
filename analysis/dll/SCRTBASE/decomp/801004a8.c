// addr: 0x801004a8  name: FUN_801004a8

undefined4 FUN_801004a8(uint *param_1,undefined4 param_2,int *param_3)

{
  int iVar1;
  
  switch(param_2) {
  case 0:
    *(short *)(param_1[0xe] + 0x42) = *(short *)(param_1[0xe] + 0x42) + 0x11;
    if (param_3 == (int *)0x0) {
      return 0;
    }
    FUN_8001d708/*0x8001d708*/(param_1[0xe]);
    if ((*param_1 & 0x80) == 0) {
      return 0;
    }
    param_3 = (int *)0x1;
    break;
  default:
    goto switchD_801004e0_caseD_1;
  case 3:
    goto switchD_801004e0_caseD_3;
  case 4:
    goto switchD_801004e0_caseD_4;
  case 8:
    goto switchD_801004e0_caseD_8;
  case 9:
    break;
  }
  if ((param_3 == (int *)0x0) || (param_3 != (int *)0x1)) {
    FUN_80020778/*0x80020778*/(param_1);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
  }
  if ((*param_1 & 0x80) != 0) {
    Stream_FatalOom/*0x80015368*/(&DAT_80100074);
  }
  if (param_1[0xe] != 0) {
    FUN_80020744/*0x80020744*/(param_1);
    **(ushort **)(param_1[0xe] + 0x30) = **(ushort **)(param_1[0xe] + 0x30) & 0xfffb | 1;
switchD_801004e0_caseD_3:
    if (*(char *)(*param_3 + 4) == '\a') {
      param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
switchD_801004e0_caseD_8:
      iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
      if (iVar1 != 0) {
        FUN_80020778/*0x80020778*/(param_1);
switchD_801004e0_caseD_4:
        FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      }
    }
  }
switchD_801004e0_caseD_1:
  return 0;
}

