// addr: 0x80100168  name: FUN_80100168

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100168(uint *param_1,int param_2)

{
  uint uVar1;
  
  if (param_2 != 1) {
    if (param_2 != 5) {
      return 0;
    }
    func_0x80044ac8(_DAT_800658fc,0x3b,param_1 + 0x12);
  }
  uVar1 = 0x80020000;
  if ((char)param_1[2] != '\0') {
    FUN_8001bddc/*0x8001bddc*/(param_1[0xc]);
    param_1[0xc] = 0;
    FUN_8001add0/*0x8001add0*/(param_1,_DAT_800737d8,0x24,8);
    uVar1 = *param_1 | 0x34;
    *param_1 = uVar1;
  }
  param_1[0x19] = uVar1 + 0x23dc;
  return 0;
}

