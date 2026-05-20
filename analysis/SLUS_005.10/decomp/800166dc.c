// addr: 0x800166dc  name: FUN_800166dc

int FUN_800166dc(undefined4 *param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  for (piVar1 = *(int **)*param_1; piVar1 != (int *)0x0; piVar1 = (int *)*piVar1) {
    iVar2 = iVar2 + 1;
  }
  return iVar2;
}

