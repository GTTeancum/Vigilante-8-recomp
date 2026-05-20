// addr: 0x800434d0  name: FUN_800434d0

void FUN_800434d0(undefined4 param_1,int param_2)

{
  gte_ldVXY0(param_2);
  gte_ldVZ0(param_2 + 4);
  FUN_8004366c();
  gte_rtv0_b();
  gte_stIR1();
  gte_stIR2();
                    /* WARNING: Treating indirect jump as return */
  gte_stIR3();
  return;
}

