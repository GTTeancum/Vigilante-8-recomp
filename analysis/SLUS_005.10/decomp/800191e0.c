// addr: 0x800191e0  name: FUN_800191e0

int FUN_800191e0(int *param_1,byte *param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  
  uVar1 = (uint)*param_2;
  iVar3 = 0;
  if (0x1f < uVar1) {
    do {
      param_2 = param_2 + 1;
      iVar2 = uVar1 - *(byte *)(*param_1 + 5);
      uVar1 = (uint)*param_2;
      iVar3 = iVar3 + (uint)*(byte *)(*param_1 + iVar2 * 5 + 0xb);
    } while (0x1f < uVar1);
  }
  return iVar3;
}

