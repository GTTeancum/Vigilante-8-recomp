// addr: 0x800200b8  name: FUN_800200b8

void FUN_800200b8(int *param_1,code *param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while ((piVar2 != (int *)0x0 && (iVar3 = (*param_2)(piVar1,param_3), iVar3 == 0))) {
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return;
}

