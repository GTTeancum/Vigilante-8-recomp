// addr: 0x8005289c  name: PRNT_OBJ_258

void PRNT_OBJ_258(void)

{
  undefined4 *unaff_s1;
  uint unaff_s3;
  undefined2 uStack00000038;
  
  if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
    *(undefined2 *)*unaff_s1 = uStack00000038;
    PRNT_OBJ_65C();
    return;
  }
  *(undefined4 *)*unaff_s1 = _uStack00000038;
  PRNT_OBJ_65C();
  return;
}

