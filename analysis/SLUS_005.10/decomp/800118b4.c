// addr: 0x800118b4  name: FUN_800118b4

void FUN_800118b4(undefined4 param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = (undefined4 *)FUN_800116f4(0xc);
  iVar1 = iRam00000004;
  puVar2[2] = param_1;
  puVar3 = (undefined4 *)(&DAT_8006eca8)[iVar1 * 3];
  (&DAT_8006eca8)[iVar1 * 3] = puVar2;
  *puVar3 = puVar2;
  puVar2[1] = puVar3;
  *puVar2 = &DAT_8006eca4 + iVar1 * 3;
  return;
}

