// addr: 0x8001afa0  name: FUN_8001afa0

short * FUN_8001afa0(int param_1,ushort param_2,short param_3)

{
  short *psVar1;
  
  while( true ) {
    if (param_2 == 0xffff) {
      return (short *)0x0;
    }
    psVar1 = (short *)(param_1 + (uint)param_2 * 0x1c + 0x1c);
    if (*psVar1 == param_3) break;
    param_2 = psVar1[0xc];
  }
  return psVar1;
}

