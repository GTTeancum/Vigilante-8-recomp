// addr: 0x80021600  name: FUN_80021600

void FUN_80021600(void)

{
  FUN_800290b4();
  FUN_8001d370();
  FUN_80041fd4();
  FUN_800215d0();
  FUN_80029750();
  (*pcRam00000730)(uRam000006f8,0x10,0);
  if (iRam000007d8 != 0) {
    FUN_80022cd0(iRam000007d8,&DAT_80065ab0,iRam0000060c + 0xffc);
  }
  return;
}

