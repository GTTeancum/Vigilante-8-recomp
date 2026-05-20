// addr: 0x8004316c  name: FUN_8004316c

undefined2 * FUN_8004316c(SVECTOR *param_1,undefined2 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldv0(param_1);
  gte_rtv0_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *param_2 = (short)uVar1;
  param_2[1] = (short)uVar2;
  param_2[2] = (short)uVar3;
  return param_2;
}

