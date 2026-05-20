// addr: 0x800202f4  name: FUN_800202f4

void FUN_800202f4(uint *param_1)

{
  if ((*param_1 & 4) != 0) {
    FUN_8001fe50(&DAT_80065a80,param_1);
  }
  if ((*param_1 & 0x80) != 0) {
    FUN_8001fe50(&DAT_80065a60,param_1);
  }
  FUN_8001fe50(&DAT_80065a18,param_1);
  return;
}

