// addr: 0x8004c1cc  name: C_011_OBJ_778

void C_011_OBJ_778(void)

{
  if ((ushort)DAT_800a32d8[3] - 1 != (uint)(ushort)DAT_800a32d8[2]) {
    if (DAT_800a32b8 != 0) {
      C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,0);
      DAT_800a32a8 = DAT_800a32a8 + 1;
      C_011_OBJ_8B0();
      return;
    }
    C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8);
    COMMON_DELAY = 0x1325;
    *DAT_800a32d8 = 3;
    DAT_800a32ac = DAT_800a32ac + 1;
    if ((DAT_800a32b8 != 0) && (DAT_800a32a4 != 0)) {
      data_ready_callback();
    }
    return;
  }
  DAT_800a32a4 = 1;
  if (DAT_800a32b8 != 0) {
    C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,1);
    DAT_800a32a8 = DAT_800a32a8 + 1;
    C_011_OBJ_814();
    return;
  }
  C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8);
  DAT_800a328c = 0;
  DAT_800a3288 = 0;
  DAT_800a32a0 = DAT_800a3298;
  C_011_OBJ_8B0();
  return;
}

