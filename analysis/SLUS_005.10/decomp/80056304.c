// addr: 0x80056304  name: PADSEQD_OBJ_D0

undefined4 PADSEQD_OBJ_D0(int param_1)

{
  undefined4 uVar1;
  
  if (*(code **)(param_1 + 0x14) != (code *)0x0) {
    (**(code **)(param_1 + 0x14))();
    uVar1 = PADSEQD_OBJ_F8();
    return uVar1;
  }
  _padSendAtLoadInfo();
  return 0;
}

