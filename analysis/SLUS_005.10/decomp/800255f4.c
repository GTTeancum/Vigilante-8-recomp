// addr: 0x800255f4  name: FUN_800255f4

undefined * FUN_800255f4(uint param_1,uint param_2)

{
  return &DAT_8008f020 +
         (uint)*(byte *)((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (param_2 >> 0x16)] +
                         (param_2 >> 0x10 & 0x3f) + (param_1 >> 10 & 0xfc0) + 0x2000) * 0x20;
}

