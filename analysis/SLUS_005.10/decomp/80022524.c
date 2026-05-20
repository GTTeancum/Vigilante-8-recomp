// addr: 0x80022524  name: FUN_80022524

uint FUN_80022524(int *param_1,int *param_2)

{
  uint uVar1;
  
  FUN_80015a20(param_1,8);
  uVar1 = param_1[1];
  uVar1 = uVar1 >> 0x18 | uVar1 >> 8 & 0xff00 | (uVar1 & 0xff00) << 8 | uVar1 << 0x18;
  param_1[1] = uVar1;
  uVar1 = uVar1 + 1 & 0xfffffffe;
  *param_2 = (*param_2 + -8) - uVar1;
  if (*param_1 == 0x4d524f46) {
    FUN_80015a20(param_1,4);
    uVar1 = 0xffffffff;
    param_1[1] = param_1[1] + -4;
  }
  return uVar1;
}

