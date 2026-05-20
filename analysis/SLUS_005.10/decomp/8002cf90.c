// addr: 0x8002cf90  name: FUN_8002cf90

bool FUN_8002cf90(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  
  uVar2 = (int)((uint)*(byte *)(param_1 + 0xb3) + param_2 + 3) % 3;
  *(char *)(param_1 + 0xb3) = (char)uVar2;
  iVar3 = 0;
  if (*(int *)(param_1 + ((uVar2 & 0xff) + 9) * 4 + 0xec) == 0) {
    iVar1 = 1;
    do {
      iVar3 = iVar1;
      if (2 < iVar3) break;
      uVar2 = (int)((uint)*(byte *)(param_1 + 0xb3) + param_2 + 3) % 3;
      *(char *)(param_1 + 0xb3) = (char)uVar2;
      iVar1 = iVar3 + 1;
    } while (*(int *)(param_1 + ((uVar2 & 0xff) + 9) * 4 + 0xec) == 0);
  }
  return iVar3 < 2;
}

