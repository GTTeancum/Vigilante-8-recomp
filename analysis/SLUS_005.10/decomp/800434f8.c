// addr: 0x800434f8  name: FUN_800434f8

void FUN_800434f8(undefined4 param_1,int param_2,undefined2 *param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldVXY0(param_2);
  gte_ldVZ0(param_2 + 4);
  FUN_8004366c();
  gte_rtv0_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *param_3 = (short)uVar1;
  param_3[1] = (short)uVar2;
                    /* WARNING: Treating indirect jump as return */
  param_3[2] = (short)uVar3;
  return;
}

