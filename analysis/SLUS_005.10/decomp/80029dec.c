// addr: 0x80029dec  name: FUN_80029dec

void FUN_80029dec(void)

{
  undefined *puVar1;
  int iVar2;
  undefined *puVar3;
  int iVar4;
  
  iVar4 = 0;
  puVar3 = &UNK_800a1e24;
  do {
    iVar2 = 0;
    puVar1 = puVar3;
    do {
      puVar1[3] = 2;
      puVar1[7] = 0x68;
      puVar1[4] = 0xff;
      puVar1[5] = 0xff;
      puVar1[6] = 0xff;
      iVar2 = iVar2 + 1;
      puVar1 = puVar1 + 0x14;
    } while (iVar2 < 0x10);
    iVar4 = iVar4 + 1;
    puVar3 = puVar3 + 0x504;
  } while (iVar4 < 2);
  return;
}

