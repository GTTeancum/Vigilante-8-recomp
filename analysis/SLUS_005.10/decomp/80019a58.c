// addr: 0x80019a58  name: FUN_80019a58

void FUN_80019a58(int *param_1,undefined4 param_2,short *param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  int unaff_s2;
  int unaff_s3;
  
  uVar1 = param_4 & 3;
  if (uVar1 == 1) {
    iVar2 = FUN_80019138(param_1,param_2);
    unaff_s2 = ((int)*param_3 + (int)param_3[2]) - iVar2;
  }
  else if (uVar1 < 2) {
    if (uVar1 == 0) {
      unaff_s2 = (int)*param_3;
    }
  }
  else if (uVar1 == 2) {
    iVar2 = FUN_80019138(param_1,param_2);
    unaff_s2 = (int)*param_3 + ((uint)(param_3[2] - iVar2) >> 1);
  }
  uVar1 = param_4 & 0xc;
  if (uVar1 == 4) {
    unaff_s3 = ((int)param_3[1] + (int)param_3[3]) - (uint)*(byte *)(*param_1 + 6);
  }
  else if (uVar1 < 5) {
    if (uVar1 == 0) {
      unaff_s3 = (int)param_3[1];
    }
  }
  else if (uVar1 == 8) {
    unaff_s3 = (int)param_3[1] + (int)((int)param_3[3] - (uint)*(byte *)(*param_1 + 6)) / 2;
  }
  if ((param_4 & 0x40) != 0) {
    iVar2 = param_1[1];
    *(undefined1 *)(param_1 + 1) = 0;
    *(undefined1 *)((int)param_1 + 5) = 0;
    *(undefined1 *)((int)param_1 + 6) = 0;
    FUN_80019960(param_1,param_2,unaff_s2 + ((int)param_4 >> 8 & 0xfU),
                 unaff_s3 + ((int)param_4 >> 0xc & 0xfU));
    param_1[1] = iVar2;
  }
  FUN_80019960(param_1,param_2,unaff_s2,unaff_s3);
  return;
}

