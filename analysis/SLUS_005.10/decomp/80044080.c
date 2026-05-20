// addr: 0x80044080  name: FUN_80044080

void FUN_80044080(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  
  iVar1 = (param_2 << 0x11) >> 0x10;
  FUN_80047674(iVar1,iVar1,param_3,param_4,0xff);
  uRam000008e4 = (short)param_3;
  uRam00000900 = (short)param_2;
  uRam000005a8 = param_1 == 0;
  return;
}

