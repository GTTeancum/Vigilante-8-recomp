// addr: 0x80043ff0  name: FUN_80043ff0

void FUN_80043ff0(int param_1,uint param_2)

{
  uint uVar1;
  
  if (param_1 != 0) {
    SPU_VOICE_KEY_ON._0_2_ = (short)param_2;
    SPU_VOICE_KEY_ON._2_2_ = (short)(param_2 >> 0x10);
    uRam000005b4 = uRam000005b4 | param_2;
    return;
  }
  uRam000005b4 = uRam000005b4 & ~param_2;
  uVar1 = uRam000005b4;
  SPU_VOICE_KEY_OFF._2_2_ = ~(ushort)(uRam000005b4 >> 0x10);
  SPU_VOICE_KEY_OFF._0_2_ = ~(ushort)uRam000005b4;
  uRam000005b4 = uVar1;
  return;
}

