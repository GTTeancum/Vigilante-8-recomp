// addr: 0x8004545c  name: SpuStart

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void SpuStart(void)

{
  if (DAT_8005edc4 == 0) {
    DAT_8005edc4 = 1;
    FUN_80053a24();
    FUN_80046014(_spu_FiDMA);
    DAT_8005ed5c = OpenEvent(0xf0000009,0x20,0x2000,(func *)0x0);
    EnableEvent(DAT_8005ed5c);
    FUN_80053a34();
  }
  return;
}

