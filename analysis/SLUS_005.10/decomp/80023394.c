// addr: 0x80023394  name: FUN_80023394

uint * FUN_80023394(int *param_1,uint param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  uint uVar3;
  uint *puVar4;
  uint uVar5;
  uint *puVar6;
  
  uVar5 = 0xffffffff;
  puVar6 = (uint *)0x0;
  piVar1 = (int *)*param_1;
  for (piVar2 = (int *)*(int *)*param_1; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
    puVar4 = (uint *)piVar1[2];
    if ((((0x1f < *(short *)((int)puVar4 + 6)) && ((*puVar4 & 0x4000) != 0)) &&
        ((*puVar4 & param_2) != 0)) && (uVar3 = FUN_80016aac(param_3,puVar4 + 0x12), uVar3 < uVar5))
    {
      uVar5 = uVar3;
      puVar6 = puVar4;
    }
    piVar1 = piVar2;
  }
  return puVar6;
}

