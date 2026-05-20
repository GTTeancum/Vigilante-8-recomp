// addr: 0x8010020c  name: FUN_8010020c

void FUN_8010020c(uint *param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  uint *puVar2;
  
  if ((param_2 == 0) || (puVar2 = param_1, param_2 != 1)) {
    *(short *)(param_1[0xe] + 0x42) = *(short *)(param_1[0xe] + 0x42) + 0x11;
    if (param_3 != 0) {
      FUN_8001d708/*0x8001d708*/(param_1[0xe],param_2);
    }
    puVar2 = (uint *)0x1;
  }
  *param_1 = *param_1 | 0x80;
  uVar1 = FUN_80017160/*0x80017160*/(puVar2);
  *(undefined2 *)(param_1[0xe] + 0x42) = uVar1;
  func_0x800223dc(param_1,param_2,param_3);
  return;
}

