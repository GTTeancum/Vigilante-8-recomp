// addr: 0x80053b78  name: StopRCnt

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

long StopRCnt(ulong param_1)

{
  _I_MASK = _I_MASK & ~*(uint *)(&DAT_8006523c + (param_1 & 0xffff) * 4);
  return 1;
}

