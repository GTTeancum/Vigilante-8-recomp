// addr: 0x80104c88  name: FUN_80104c88

void FUN_80104c88(short param_1,undefined2 param_2,int param_3,int param_4,int param_5,int param_6)

{
  short sVar1;
  short sVar2;
  
  *(short *)(param_6 + 0x10) = param_1;
  *(undefined2 *)(param_6 + 0x12) = param_2;
  sVar1 = (short)param_4;
  sVar2 = (short)param_3;
  if (param_4 < param_3) {
    sVar2 = sVar1;
  }
  *(short *)(param_6 + 0x18) = sVar2;
  DrawPrim/*0x8004fb18*/();
  if (param_4 < param_3) {
    *(short *)(param_5 + 0x10) = param_1 + sVar1;
    *(undefined2 *)(param_5 + 0x12) = param_2;
    *(short *)(param_5 + 0x18) = (short)param_3 - sVar1;
    DrawPrim/*0x8004fb18*/();
  }
  return;
}

