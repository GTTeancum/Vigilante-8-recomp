// addr: 0x80050a7c  name: SYS_OBJ_1878

uint SYS_OBJ_1878(byte *param_1)

{
  uint uVar1;
  
  if (param_1 == (byte *)0x0) {
    uVar1 = SYS_OBJ_18F0();
    return uVar1;
  }
  return (uint)(param_1[2] >> 3) << 0xf | (uint)(*param_1 >> 3) << 10 | 0xe2000000 |
         ((int)(-(int)*(short *)(param_1 + 6) & 0xffU) >> 3) << 5 |
         (int)(-(int)*(short *)(param_1 + 4) & 0xffU) >> 3;
}

