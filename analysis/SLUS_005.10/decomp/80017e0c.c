// addr: 0x80017e0c  name: FUN_80017e0c

void FUN_80017e0c(void)

{
  if (iRam000006c4 != 0) {
    uRam000006c8 = 0;
    FUN_80017db4();
    iRam000006c4 = 0;
  }
  return;
}

