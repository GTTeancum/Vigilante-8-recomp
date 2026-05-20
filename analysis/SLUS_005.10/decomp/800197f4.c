// addr: 0x800197f4  name: FUN_800197f4

void FUN_800197f4(int *param_1,undefined4 param_2,short *param_3,uint param_4)

{
  int iVar1;
  uint uVar2;
  int in_t0;
  int unaff_s1;
  
  uVar2 = param_4 & 3;
  if (uVar2 == 1) {
    iVar1 = FUN_80019138(param_1,param_2);
    in_t0 = ((int)*param_3 + (int)param_3[2]) - iVar1;
  }
  else if (uVar2 < 2) {
    if (uVar2 == 0) {
      in_t0 = (int)*param_3;
    }
  }
  else if (uVar2 == 2) {
    iVar1 = FUN_80019138(param_1,param_2);
    in_t0 = (int)*param_3 + ((uint)(param_3[2] - iVar1) >> 1);
  }
  param_4 = param_4 & 0xc;
  if (param_4 == 4) {
    unaff_s1 = ((int)param_3[1] + (int)param_3[3]) - (uint)*(byte *)(*param_1 + 6);
  }
  else if (param_4 < 5) {
    if (param_4 == 0) {
      unaff_s1 = (int)param_3[1];
    }
  }
  else if (param_4 == 8) {
    unaff_s1 = (int)param_3[1] + (int)((int)param_3[3] - (uint)*(byte *)(*param_1 + 6)) / 2;
  }
  FUN_80019458(param_1,param_2,in_t0,unaff_s1);
  return;
}

