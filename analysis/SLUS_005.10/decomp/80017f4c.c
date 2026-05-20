// addr: 0x80017f4c  name: FUN_80017f4c

void FUN_80017f4c(short *param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar1 = FUN_80017e3c(param_1,0,(int)*param_1,(int)param_1[1],(int)param_1[2],param_2);
  uVar2 = FUN_80017e3c(param_1,0,(int)*param_1,param_1[1] + param_2,(int)param_1[2],
                       param_1[3] - param_2);
  param_1[4] = 3;
  param_1[5] = 0;
  *(undefined4 *)(param_1 + 8) = uVar1;
  *(undefined4 *)(param_1 + 10) = uVar2;
  return;
}

