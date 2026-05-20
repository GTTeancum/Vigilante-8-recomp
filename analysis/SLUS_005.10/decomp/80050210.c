// addr: 0x80050210  name: SYS_OBJ_100C

void SYS_OBJ_100C(uint param_1,uint param_2)

{
  int in_v0;
  uint in_v1;
  uint unaff_s0;
  uint unaff_s2;
  
  if (in_v0 != 0) {
    param_2 = unaff_s2;
  }
  SYS_OBJ_20DC((in_v1 & 0xfff) << 0xc | param_1 & 0xfff | 0x6000000);
  SYS_OBJ_20DC((param_2 & 0x3ff) << 10 | unaff_s0 & 0x3ff | 0x7000000);
  FUN_80044c44(&DAT_80065090);
  return;
}

