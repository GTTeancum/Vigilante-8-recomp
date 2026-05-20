// addr: 0x80100c18  name: FUN_80100c18

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c18(int param_1,int param_2)

{
  if (param_2 != 0) {
    if (param_2 == 5) goto LAB_80100c4c;
    param_1 = 1;
  }
  _DAT_00000029 = *(int *)(param_1 + 0x28) + -0x5f5;
LAB_80100c4c:
  FUN_800205f8/*0x800205f8*/();
  return 0;
}

