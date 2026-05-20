// addr: 0x80012088  name: FUN_80012088

uint FUN_80012088(uint param_1,undefined4 param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = 0;
  for (; (param_1 & 0xffff) != 0; param_1 = param_1 >> 1) {
    uVar1 = uVar1 | (param_1 & 0x10001) << (param_3 & 0xf);
    param_3 = param_3 >> 4 | param_4 << 0x1c;
    param_4 = param_4 >> 4;
  }
  return uVar1;
}

