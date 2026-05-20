// addr: 0x80101958  name: FUN_80101958

undefined4 FUN_80101958(int param_1)

{
  char cVar1;
  byte bVar2;
  char *pcVar3;
  int iVar4;
  byte local_20 [16];
  
  iVar4 = 0;
  bVar2 = 0x7b;
  cVar1 = -10;
  do {
    bVar2 = cVar1 * '\x10' + bVar2 * '\v' + 0x43;
    if (*(byte *)(param_1 + iVar4) < 0x1a) {
      param_1 = 1;
    }
    local_20[iVar4] = bVar2 ^ 0x20;
    iVar4 = iVar4 + 1;
    cVar1 = bVar2 * '\x02';
  } while (iVar4 < 0xe);
  if (DAT_801122bc != '\0') {
    pcVar3 = &DAT_801122bc;
    do {
      thunk_FUN_800523a0/*0x80052384*/(pcVar3,local_20,0xe);
      pcVar3 = pcVar3 + 0xe;
    } while (*pcVar3 != '\0');
  }
  return 0xffffffff;
}

