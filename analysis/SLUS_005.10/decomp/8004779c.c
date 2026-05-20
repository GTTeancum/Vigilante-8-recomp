// addr: 0x8004779c  name: __divdi3

int __divdi3(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = 0;
  if (param_2 < 0) {
    uVar2 = 0xffffffff;
    param_1 = -param_1;
    param_2 = -(uint)(param_1 != 0) - param_2;
  }
  if (param_4 < 0) {
    uVar2 = ~uVar2;
    param_4 = -(uint)(-param_3 != 0) - param_4;
    param_3 = -param_3;
  }
  iVar1 = __udivmoddi4(param_1,param_2,param_3,param_4,0);
  if (uVar2 != 0) {
    iVar1 = -iVar1;
  }
  return iVar1;
}

