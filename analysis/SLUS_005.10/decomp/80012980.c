// addr: 0x80012980  name: FUN_80012980

void FUN_80012980(void)

{
  if (iRam00000608 != 0) {
    FUN_80045088();
    iRam00000608 = 0;
  }
  return;
}

