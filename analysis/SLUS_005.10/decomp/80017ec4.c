// addr: 0x80017ec4  name: FUN_80017ec4

void FUN_80017ec4(short *param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar1 = FUN_80017e3c(param_1,0,(int)*param_1,(int)param_1[1],param_2,(int)param_1[3]);
  uVar2 = FUN_80017e3c(param_1,0,*param_1 + param_2,(int)param_1[1],param_1[2] - param_2,
                       (int)param_1[3]);
  param_1[4] = 2;
  param_1[5] = 0;
  *(undefined4 *)(param_1 + 8) = uVar1;
  *(undefined4 *)(param_1 + 10) = uVar2;
  return;
}

