// addr: 0x8010a614  name: FUN_8010a614

void FUN_8010a614(void)

{
  DAT_801133b1 = 0;
  DAT_801133b0 = 0;
  DAT_801133a0 = Heap_CallocOrRetry/*0x8001178c*/(0x28,0x20);
  DAT_801133a4 = Heap_CallocOrRetry/*0x8001178c*/(0x200,0x20);
  return;
}

