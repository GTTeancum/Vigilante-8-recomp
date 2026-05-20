// addr: 0x8001b36c  name: FUN_8001b36c

int * FUN_8001b36c(int *param_1,uint param_2)

{
  int *piVar1;
  
  piVar1 = param_1 + (param_2 & 0xffff) * 3 + 3;
  if ((short)*piVar1 == 0) {
    FUN_800187e4(*(undefined4 *)((param_2 & 0xffff) * 4 + *(int *)(*param_1 + 0x14)),piVar1);
  }
  return piVar1;
}

