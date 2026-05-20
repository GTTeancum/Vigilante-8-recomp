// addr: 0x8004ef30  name: PATCHGTE_OBJ_DC

uint PATCHGTE_OBJ_DC(u_long *param_1,int param_2,int param_3,int param_4,int param_5,int param_6,
                    short param_7)

{
  u_short uVar1;
  uint uVar2;
  RECT RStack_20;
  
  RStack_20.x = (short)param_4;
  RStack_20.h = param_7;
  RStack_20.y = (short)param_5;
  if (param_2 == 1) {
    RStack_20.w = (short)(param_6 / 2);
    uVar2 = EXT_OBJ_A8();
    return uVar2;
  }
  if (param_2 < 2) {
    if (param_2 != 0) {
      uVar2 = EXT_OBJ_AC();
      return uVar2;
    }
    if (param_6 < 0) {
      param_6 = param_6 + 3;
    }
    RStack_20.w = (short)(param_6 >> 2);
    uVar2 = EXT_OBJ_A8(&RStack_20);
    return uVar2;
  }
  if (param_2 != 2) {
    uVar2 = EXT_OBJ_AC(&RStack_20);
    return uVar2;
  }
  RStack_20.w = (short)param_6;
  LoadImage(&RStack_20,param_1);
  uVar1 = GetTPage(2,param_3,param_4,param_5);
  return (uint)uVar1;
}

