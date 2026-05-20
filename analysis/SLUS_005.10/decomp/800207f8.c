// addr: 0x800207f8  name: FUN_800207f8

undefined4 FUN_800207f8(uint *param_1)

{
  undefined4 uVar1;
  
  if ((*param_1 & 4) == 0) {
    uVar1 = 0;
  }
  else {
    *param_1 = *param_1 & 0xfffffffb;
    uVar1 = FUN_8001fe8c(&DAT_80065a80);
  }
  return uVar1;
}

