// addr: 0x800543c0  name: FUN_800543c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_800543c0(void)

{
  DAT_80065294 = 0;
  FUN_80053a24();
  SysDeqIntRP(2,&DAT_800a4c68);
  SysEnqIntRP(2,&DAT_800a4c68);
  _I_STAT = 0xfffffffe;
  _I_MASK = _I_MASK | 1;
  ChangeClearRCnt(3,0);
  FUN_80053a34();
  (*DAT_80065260)(DAT_80065290);
  (*DAT_80065260)(DAT_80065290 + 0xf0);
  DAT_800a4c7c = 0;
  DAT_800a4c78 = 0;
  DAT_80065294 = 1;
  return;
}

