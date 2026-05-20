// addr: 0x8001356c  name: FUN_8001356c

void FUN_8001356c(int param_1)

{
  int iVar1;
  
  while (iRam000005ac != 4) {
    PutDrawEnv((DRAWENV *)(&DAT_8006f208 + (1 - iRam00000008) * 0x5c));
    FUN_8001a0ac(&DAT_80065520,0);
    FUN_80019f9c(&DAT_80065520,0xffffff);
    *(undefined1 *)(param_1 + 4) = 0x80;
    *(undefined1 *)(param_1 + 5) = 0x80;
    *(undefined1 *)(param_1 + 6) = 0;
    FUN_80019010(param_1,1);
    FUN_80019a58(param_1,"Insert Original CD",&DAT_80065528,10);
    do {
      VSync(0);
      iVar1 = FUN_80043bb4();
    } while (iVar1 == 0);
    FUN_8001a0ac(&DAT_80065528,0);
    FUN_80019a58(param_1,"CD COVER OPEN",&DAT_80065528,10);
    do {
      iVar1 = FUN_80043bb4();
    } while (iVar1 != 0);
    FUN_8001a0ac(&DAT_80065528,0);
    FUN_80019a58(param_1,"CHECKING CD",&DAT_80065528,10);
    FUN_80043aec();
  }
  return;
}

