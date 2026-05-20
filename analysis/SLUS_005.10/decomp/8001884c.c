// addr: 0x8001884c  name: FUN_8001884c

undefined4 FUN_8001884c(short *param_1)

{
  ushort uVar1;
  ushort uVar2;
  undefined4 uVar3;
  
  if (*param_1 == 0) {
    uVar3 = 1;
  }
  else {
    uVar1 = param_1[4];
    uVar2 = param_1[3];
    if (param_1[5] != 0) {
      FUN_8001859c(param_1[5]);
    }
    *param_1 = 0;
    uVar3 = FUN_80018530((uVar1 & 0xf) * 0x40 + ((int)(uVar2 & 0xff) >> (2 - (uVar1 >> 7) & 0x1f)),
                         (uVar1 & 0x10) * 0x10 + (uint)(uVar2 >> 8));
  }
  return uVar3;
}

