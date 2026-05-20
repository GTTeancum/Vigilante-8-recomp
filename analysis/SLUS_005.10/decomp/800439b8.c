// addr: 0x800439b8  name: FUN_800439b8

void FUN_800439b8(undefined4 *param_1,int param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldR11R12(*param_1);
  gte_ldR13R21(param_1[1]);
  gte_ldR22R23(param_1[2]);
  gte_ldR31R32(param_1[3]);
  gte_ldR33(param_1[4]);
  gte_ldIR1(0x1000);
  gte_ldIR2(param_4);
  gte_ldIR3(-param_3);
  gte_rtir_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(-param_4,0x1000,param_2);
  gte_rtir_b();
  *(short *)param_1 = (short)uVar1;
  *(short *)((int)param_1 + 6) = (short)uVar2;
  *(short *)(param_1 + 3) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(param_3,-param_2,0x1000);
  gte_rtir_b();
  *(short *)((int)param_1 + 2) = (short)uVar1;
  *(short *)(param_1 + 2) = (short)uVar2;
  *(short *)((int)param_1 + 0xe) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *(short *)(param_1 + 1) = (short)uVar1;
  *(short *)((int)param_1 + 10) = (short)uVar2;
  *(short *)(param_1 + 4) = (short)uVar3;
  return;
}

