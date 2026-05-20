// addr: 0x80053b48  name: StartRCnt

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

long StartRCnt(ulong param_1)

{
  _I_MASK = _I_MASK | *(uint *)(&DAT_8006523c + (param_1 & 0xffff) * 4);
  return (uint)((param_1 & 0xffff) < 3);
}

