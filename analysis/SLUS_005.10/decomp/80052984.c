// addr: 0x80052984  name: PRNT_OBJ_340

void PRNT_OBJ_340(undefined4 param_1)

{
  uint unaff_s3;
  
  if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
    PRNT_OBJ_3CC(param_1,10);
    return;
  }
  PRNT_OBJ_3C8(param_1,10);
  return;
}

