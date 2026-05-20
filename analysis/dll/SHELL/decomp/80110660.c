// addr: 0x80110660  name: FUN_80110660

void FUN_80110660(void)

{
  undefined1 *puVar1;
  int iVar2;
  undefined1 local_11;
  
  if (DAT_80113438 != 0) {
    printf/*0x80052604*/(s_Access_Denied____system_busy_8010124c);
  }
  do {
    iVar2 = 0x7f;
    puVar1 = &local_11;
    do {
      *puVar1 = 0xff;
      iVar2 = iVar2 + -1;
      puVar1 = puVar1 + -1;
    } while (-1 < iVar2);
    iVar2 = 0;
    while (iVar2 = iVar2 + 1, puVar1 == (undefined1 *)0x0) {
      puVar1 = (undefined1 *)0x1;
      if (0xe < iVar2) {
        return;
      }
    }
  } while( true );
}

