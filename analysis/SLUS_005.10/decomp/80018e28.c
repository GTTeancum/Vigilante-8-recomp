// addr: 0x80018e28  name: FUN_80018e28

int * FUN_80018e28(int *param_1)

{
  int *piVar1;
  uint *puVar2;
  int iVar3;
  uint *puVar4;
  
  iVar3 = ((*param_1 + -4) - (int)param_1) + (*(byte *)(*param_1 + 3) + 1) * 4;
  piVar1 = (int *)FUN_800116f4(iVar3 + 4);
  puVar4 = (uint *)(piVar1 + 1);
  FUN_80044c44(puVar4,param_1 + 1,iVar3);
  puVar2 = (uint *)((int)piVar1 + (*param_1 - (int)param_1));
  *piVar1 = (int)puVar2;
  if (puVar4 != puVar2) {
    do {
      puVar2 = puVar4 + *(byte *)((int)puVar4 + 3) + 1;
      *puVar4 = *puVar4 & 0xff000000 | (uint)puVar2 & 0xffffff;
      puVar4 = puVar2;
    } while (puVar2 != (uint *)*piVar1);
  }
  return piVar1;
}

