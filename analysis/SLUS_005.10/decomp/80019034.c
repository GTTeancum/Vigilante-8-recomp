// addr: 0x80019034  name: FUN_80019034

int * FUN_80019034(int *param_1,undefined4 param_2)

{
  int *piVar1;
  
  piVar1 = (int *)FUN_800116f4(0x14);
  FUN_800187e4((int)param_1 + *param_1,piVar1 + 2);
  *piVar1 = (int)param_1;
  *(undefined1 *)(piVar1 + 1) = 0x80;
  *(undefined1 *)((int)piVar1 + 5) = 0x80;
  *(undefined1 *)((int)piVar1 + 6) = 0x80;
  FUN_80019010(piVar1,param_2);
  return piVar1;
}

