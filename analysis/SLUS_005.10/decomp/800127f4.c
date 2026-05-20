// addr: 0x800127f4  name: FUN_800127f4

void FUN_800127f4(void)

{
  uRam0000016c = 1;
  DrawSyncCallback((func *)0x0);
  iRam000005e0 = VSyncCallback(FUN_80012710);
  return;
}

