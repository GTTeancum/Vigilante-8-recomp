// addr: 0x80011c58  name: FUN_80011c58

void FUN_80011c58(undefined1 *param_1)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  int iVar3;
  
  uRam00000015 = *param_1;
  uRam000005f4 = param_1[1];
  iVar3 = 0;
  puVar2 = param_1;
  do {
    puVar1 = &DAT_80065674 + iVar3;
    iVar3 = iVar3 + 1;
    *puVar1 = puVar2[2];
    puVar2 = param_1 + iVar3;
  } while (iVar3 < 6);
  iVar3 = 0;
  puVar2 = param_1;
  do {
    puVar1 = (undefined1 *)((int)&DAT_8006567c + iVar3);
    iVar3 = iVar3 + 1;
    *puVar1 = puVar2[8];
    puVar2 = param_1 + iVar3;
  } while (iVar3 < 4);
  return;
}

