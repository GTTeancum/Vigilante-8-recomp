// addr: 0x80055e10  name: PADPORTD_OBJ_23C

/* WARNING: Removing unreachable block (ram,0x80055dcc) */
/* WARNING: Removing unreachable block (ram,0x80055dec) */
/* WARNING: Removing unreachable block (ram,0x80055df4) */
/* WARNING: Removing unreachable block (ram,0x80055e00) */

void PADPORTD_OBJ_23C(void)

{
  int unaff_s1;
  
  DAT_800652a0 = 0;
  JOY_MCD_CTRL = 0;
  DAT_8006529c = DAT_8006529c + 1;
  if (1 < DAT_8006529c) {
    return;
  }
  FUN_800544d0(DAT_8006529c * 0xf0 + unaff_s1);
  PADPORTD_OBJ_298();
  return;
}

