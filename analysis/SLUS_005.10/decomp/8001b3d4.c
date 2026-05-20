// addr: 0x8001b3d4  name: FUN_8001b3d4

int * FUN_8001b3d4(int *param_1,uint param_2)

{
  short sVar1;
  int *piVar2;
  
  piVar2 = param_1 + (param_2 & 0xffff) * 3 + 3;
  sVar1 = (short)*piVar2 + 1;
  *(short *)piVar2 = sVar1;
  if (sVar1 == 1) {
    FUN_800187e4(*(undefined4 *)((param_2 & 0xffff) * 4 + *(int *)(*param_1 + 0x14)),piVar2);
  }
  return piVar2;
}

