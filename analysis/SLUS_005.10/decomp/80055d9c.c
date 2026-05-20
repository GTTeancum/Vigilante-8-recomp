// addr: 0x80055d9c  name: PADPORTD_OBJ_1C8

void PADPORTD_OBJ_1C8(int param_1)

{
  int iVar1;
  
  iVar1 = DAT_8006529c * 0xf0;
  if (param_1 != -9) {
    if (param_1 == 0) {
      *(undefined4 *)(&DAT_800652b4 + DAT_8006529c * 4) = 0;
      PADPORTD_OBJ_23C();
      return;
    }
    _dirFailAuto(&DAT_800a4d28 + iVar1);
    PADPORTD_OBJ_2BC(&DAT_800a4d28 + iVar1);
  }
  DAT_800652a0 = 0;
  JOY_MCD_CTRL = 0;
  DAT_8006529c = DAT_8006529c + 1;
  if (1 < DAT_8006529c) {
    return;
  }
  FUN_800544d0(&DAT_800a4d28 + DAT_8006529c * 0xf0);
  PADPORTD_OBJ_298();
  return;
}

