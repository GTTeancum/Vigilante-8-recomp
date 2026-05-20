// addr: 0x80050910  name: SYS_OBJ_170C

uint SYS_OBJ_170C(int param_1,int param_2,uint param_3)

{
  uint uVar1;
  
  uVar1 = 0xe1000000;
  if (param_2 != 0) {
    uVar1 = 0xe1000200;
  }
  param_3 = param_3 & 0x9ff;
  if (param_1 != 0) {
    param_3 = param_3 | 0x400;
  }
  return uVar1 | param_3;
}

