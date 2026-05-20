// addr: 0x80044fbc  name: FUN_80044fbc

void FUN_80044fbc(int *param_1,uint param_2)

{
  uint uVar1;
  
  if ((param_1 != (int *)0x0) && (uVar1 = param_2 & 0xfffffff8, param_2 != 0)) {
    *(int **)((int)param_1 + (uVar1 - 8)) = param_1;
    *(undefined4 *)((int)param_1 + (uVar1 - 4)) = 0;
    *param_1 = (int)param_1 + (uVar1 - 8);
    DAT_8005ed4c = param_1;
    DAT_8005ed50 = param_1;
    param_1[1] = (param_2 >> 3) - 1;
  }
  return;
}

