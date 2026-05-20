// addr: 0x80100c50  name: FUN_80100c50

undefined4 FUN_80100c50(uint *param_1,int param_2)

{
  if (param_2 == 0) {
    FUN_80017324/*0x80017324*/();
    if ((*param_1 & 2) == 0) {
      param_1[0x21] = param_1[0x21] - 0x1680;
    }
    param_1[0x12] = param_1[9];
    param_1[0x13] = param_1[10];
    param_1[0x14] = param_1[0xb];
  }
  return 0;
}

