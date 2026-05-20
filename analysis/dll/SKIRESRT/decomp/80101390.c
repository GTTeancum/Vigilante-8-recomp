// addr: 0x80101390  name: FUN_80101390

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80101390(int param_1)

{
  undefined4 *puVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  
  puVar1 = (undefined4 *)FUN_80022c54/*0x80022c54*/();
  piVar4 = *(int **)(_DAT_800659fc + 0x80);
  if ((int *)*piVar4 != (int *)0x0) {
    piVar2 = (int *)*piVar4;
    do {
      piVar3 = piVar2;
      if (*(int *)(param_1 + 0x50) <= *(int *)(piVar4[2] + 0x50)) break;
      piVar2 = (int *)*piVar3;
      piVar4 = piVar3;
    } while ((int *)*piVar3 != (int *)0x0);
  }
  piVar2 = (int *)piVar4[1];
  *piVar2 = (int)puVar1;
  piVar4[1] = (int)puVar1;
  *puVar1 = piVar4;
  puVar1[1] = piVar2;
  return;
}

