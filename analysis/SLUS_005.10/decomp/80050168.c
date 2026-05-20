// addr: 0x80050168  name: SYS_OBJ_F64

void SYS_OBJ_F64(uint param_1,uint param_2)

{
  uint in_v1;
  int unaff_s2;
  
  if ((int)(param_2 + 2) <= unaff_s2) {
    SYS_OBJ_100C(param_1,0x131);
    return;
  }
  SYS_OBJ_20DC((in_v1 & 0xfff) << 0xc | param_1 & 0xfff | 0x6000000);
  SYS_OBJ_20DC((param_2 + 2 & 0x3ff) << 10 | param_2 & 0x3ff | 0x7000000);
  FUN_80044c44(&DAT_80065090);
  return;
}

