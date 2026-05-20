// addr: 0x80053c44  name: PadChkMtap

uint PadChkMtap(int param_1)

{
  uint uVar1;
  
  if (DAT_800652a8 == 0) {
    uVar1 = PADENTRY_OBJ_A4();
    return uVar1;
  }
  return (uint)(*(char *)((param_1 >> 4) * 0xf0 + DAT_80065290 + 0xe8) == '\b');
}

