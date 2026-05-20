// addr: 0x80022c54  name: FUN_80022c54

void FUN_80022c54(int param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = *piRam0000076c;
  *(undefined **)(iVar2 + 4) = &DAT_80065a70;
  piVar1 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar2;
  *piVar1 = param_1;
  return;
}

