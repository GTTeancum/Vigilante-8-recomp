// addr: 0x80055e6c  name: PADPORTD_OBJ_298

void PADPORTD_OBJ_298(undefined4 param_1,int param_2)

{
  int in_v0;
  int iVar1;
  int unaff_s1;
  int unaff_s2;
  int unaff_s3;
  
  while( true ) {
    if (in_v0 != 0) {
      return;
    }
    iVar1 = DAT_8006529c * 0xf0 + unaff_s1;
    if (param_2 != unaff_s3) {
      if (param_2 == 0) {
        *(undefined4 *)(DAT_8006529c * 4 + unaff_s2) = 0;
        PADPORTD_OBJ_23C();
        return;
      }
      _dirFailAuto(iVar1);
      PADPORTD_OBJ_2BC(iVar1);
    }
    DAT_800652a0 = 0;
    JOY_MCD_CTRL = 0;
    DAT_8006529c = DAT_8006529c + 1;
    in_v0 = 1;
    if (DAT_8006529c < 2) break;
    param_2 = 0xffff;
  }
  FUN_800544d0(DAT_8006529c * 0xf0 + unaff_s1);
  PADPORTD_OBJ_298();
  return;
}

