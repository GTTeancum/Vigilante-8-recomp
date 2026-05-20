// addr: 0x80012710  name: FUN_80012710

void FUN_80012710(void)

{
  uRam00000168 = 1;
  PutDispEnv(pDRam000005d8);
  DrawOTag(puRam000005dc);
  VSyncCallback(pfRam000005e0);
  if (pfRam000005e0 != (f *)0x0) {
    (*pfRam000005e0)();
  }
  return;
}

