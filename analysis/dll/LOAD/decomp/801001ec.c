// addr: 0x801001ec  name: FUN_801001ec

void FUN_801001ec(int *param_1,int *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = *param_1;
  if (iVar1 != 1) {
    if (iVar1 != 0) {
      if (iVar1 == 2) {
        return;
      }
      param_1 = (int *)0x1;
    }
    puVar2 = (undefined4 *)param_1[3];
    param_1[3] = (int)param_2;
    *puVar2 = param_2;
    param_2[1] = (int)puVar2;
    *param_2 = (int)(param_1 + 2);
  }
  return;
}

