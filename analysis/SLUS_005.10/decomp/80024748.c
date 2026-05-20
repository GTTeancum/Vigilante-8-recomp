// addr: 0x80024748  name: FUN_80024748

int * FUN_80024748(int *param_1,int *param_2)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)0x0;
  for (piVar2 = param_1; (piVar2 != (int *)0x0 && (piVar2[5] < param_2[5])); piVar2 = (int *)*piVar2
      ) {
    piVar1 = piVar2;
  }
  *param_2 = (int)piVar2;
  if (piVar1 != (int *)0x0) {
    *piVar1 = (int)param_2;
    return param_1;
  }
  return param_2;
}

