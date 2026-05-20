// addr: 0x80100be8  name: FUN_80100be8

undefined4 FUN_80100be8(undefined4 *param_1,int param_2,int param_3)

{
  undefined1 uVar1;
  
  if (param_2 != 0) {
    uVar1 = 3;
    if (param_2 == 1) goto LAB_80100c34;
    param_1 = (undefined4 *)0x1;
  }
  *(short *)(param_1 + 0x10) = *(short *)(param_1 + 0x10) + 0x5b;
  *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 0x44;
  if (param_3 == 0) {
    return 0;
  }
  uVar1 = FUN_8001d708/*0x8001d708*/();
  param_1 = (undefined4 *)0x1;
LAB_80100c34:
  *(undefined1 *)(param_1 + 1) = uVar1;
  *param_1 = 0x84;
  return 0;
}

