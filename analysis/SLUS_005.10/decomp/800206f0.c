// addr: 0x800206f0  name: FUN_800206f0

void FUN_800206f0(int *param_1)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  for (piVar2 = (int *)*(int *)*param_1; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
    FUN_8001de08(piVar1[2]);
    piVar1 = piVar2;
  }
  return;
}

