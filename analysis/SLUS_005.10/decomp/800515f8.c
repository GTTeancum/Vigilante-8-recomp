// addr: 0x800515f8  name: SYS_OBJ_23F4

uint SYS_OBJ_23F4(void)

{
  undefined4 unaff_s2;
  undefined4 unaff_s3;
  
  *(undefined4 *)(&DAT_800a3450 + DAT_8006514c * 0x60) = unaff_s2;
  *(undefined4 *)(&DAT_800a3448 + DAT_8006514c * 0x60) = unaff_s3;
  DAT_8006514c = DAT_8006514c + 1 & 0x3f;
  SetIntrMask(DAT_80065154);
  SYS_OBJ_24A0();
  return DAT_8006514c - DAT_80065150 & 0x3f;
}

