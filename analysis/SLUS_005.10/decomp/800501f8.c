// addr: 0x800501f8  name: SYS_OBJ_FF4

void SYS_OBJ_FF4(uint param_1,uint param_2)

{
  uint in_v1;
  uint uVar1;
  uint unaff_s2;
  
  uVar1 = param_2 + 2;
  if (((int)(param_2 + 2) <= (int)unaff_s2) && (uVar1 = 0x102, (int)unaff_s2 < 0x103)) {
    uVar1 = unaff_s2;
  }
  SYS_OBJ_20DC((in_v1 & 0xfff) << 0xc | param_1 & 0xfff | 0x6000000);
  SYS_OBJ_20DC((uVar1 & 0x3ff) << 10 | param_2 & 0x3ff | 0x7000000);
  FUN_80044c44(&DAT_80065090);
  return;
}

