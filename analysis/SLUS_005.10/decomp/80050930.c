// addr: 0x80050930  name: SYS_OBJ_172C

uint SYS_OBJ_172C(uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = 0;
  if ((-1 < (short)param_1) &&
     (uVar1 = DAT_80065028 - 1, (int)(short)param_1 <= (short)DAT_80065028 + -1)) {
    uVar1 = param_1;
  }
  if ((short)param_2 < 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = param_2 & 0x3ff;
    if ((short)DAT_8006502a + -1 < (int)(short)param_2) {
      uVar1 = SYS_OBJ_17A8(uVar1,DAT_8006502a - 1);
      return uVar1;
    }
  }
  return uVar2 << 10 | uVar1 & 0x3ff | 0xe3000000;
}

