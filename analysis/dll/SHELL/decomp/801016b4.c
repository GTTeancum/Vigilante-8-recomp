// addr: 0x801016b4  name: FUN_801016b4

undefined4 FUN_801016b4(void)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  
  pcVar1 = (char *)0x1;
  iVar2 = 0;
  do {
    iVar3 = iVar2 + 1;
    *pcVar1 = *pcVar1 + 'A';
    pcVar1 = (char *)(iVar2 + 2);
    iVar2 = iVar3;
  } while (iVar3 < 0xe);
  DAT_0000000f = 0;
  return 1;
}

