// addr: 0x80019138  name: FUN_80019138

int FUN_80019138(int *param_1,byte *param_2)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = 0;
  iVar3 = 0;
  bVar1 = *param_2;
  while (uVar2 = (uint)bVar1, uVar2 != 0) {
    param_2 = param_2 + 1;
    if (uVar2 < 0x20) {
      param_2 = param_2 + (byte)(&DAT_800568dc)[uVar2];
    }
    else {
      iVar3 = *param_1 + (uVar2 - *(byte *)(*param_1 + 5)) * 5 + 8;
      iVar4 = iVar4 + (uint)*(byte *)(iVar3 + 3);
    }
    bVar1 = *param_2;
  }
  if (iVar3 != 0) {
    return (iVar4 - (uint)*(byte *)(iVar3 + 3)) + (int)*(char *)(iVar3 + 4) +
           (uint)*(byte *)(iVar3 + 2);
  }
  return iVar4;
}

