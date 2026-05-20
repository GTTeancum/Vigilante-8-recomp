// addr: 0x80011914  name: FUN_80011914

void FUN_80011914(int param_1)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)(&DAT_8006eca0)[param_1 * 3];
  for (piVar2 = (int *)*(int *)(&DAT_8006eca0)[param_1 * 3]; piVar2 != (int *)0x0;
      piVar2 = (int *)*piVar2) {
    FUN_80045088(piVar1[2]);
    FUN_80045088(piVar1);
    piVar1 = piVar2;
  }
  (&DAT_8006eca0)[param_1 * 3] = &DAT_8006eca4 + param_1 * 3;
  (&DAT_8006eca4)[param_1 * 3] = 0;
  (&DAT_8006eca8)[param_1 * 3] = &DAT_8006eca0 + param_1 * 3;
  return;
}

