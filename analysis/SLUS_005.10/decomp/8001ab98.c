// addr: 0x8001ab98  name: FUN_8001ab98

int FUN_8001ab98(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = *(int *)(param_1 + 4);
  iVar3 = 0;
  if ((iVar2 != 0) && (iVar1 = *(int *)(iVar2 + (param_2 & 0xffff) * 4 + 4), iVar1 != 0)) {
    iVar3 = iVar2 + iVar1;
  }
  return iVar3;
}

