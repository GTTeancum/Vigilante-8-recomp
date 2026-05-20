// addr: 0x80020778  name: FUN_80020778

undefined4 FUN_80020778(uint *param_1)

{
  undefined4 uVar1;
  
  if ((*param_1 & 0x80) == 0) {
    uVar1 = 0;
  }
  else {
    *param_1 = *param_1 & 0xffffff7f;
    uVar1 = FUN_8001fe8c(&DAT_80065a60);
  }
  return uVar1;
}

