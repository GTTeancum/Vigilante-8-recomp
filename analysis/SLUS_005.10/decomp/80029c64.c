// addr: 0x80029c64  name: FUN_80029c64

undefined * FUN_80029c64(uint param_1,uint param_2)

{
  return &DAT_8008f020 +
         (uint)*(byte *)((&DAT_800911a0)[(param_1 >> 6) * 0x20 + (param_2 >> 6)] +
                         (param_2 & 0x3f) + (param_1 & 0x3f) * 0x40 + 0x2000) * 0x20;
}

