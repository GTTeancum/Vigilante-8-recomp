// addr: 0x800288fc  name: FUN_800288fc

void FUN_800288fc(uint param_1,int param_2,int param_3,undefined4 param_4,uint param_5)

{
  int iVar1;
  
  iVar1 = 1 << (param_1 & 0x1f);
  if ((param_5 & 2) == 0) {
    for (; param_2 < param_3; param_2 = param_2 + iVar1) {
      FUN_8002623c(param_1,param_2,param_4,param_5);
      param_5 = param_5 & 0xfffffffe;
    }
  }
  else {
    iVar1 = param_3 - iVar1;
    FUN_800288fc(param_1,param_2,iVar1,param_4,param_5 & 0xfffffffd);
    if (param_2 < param_3) {
      FUN_8002623c(param_1,iVar1,param_4,param_5);
    }
  }
  return;
}

