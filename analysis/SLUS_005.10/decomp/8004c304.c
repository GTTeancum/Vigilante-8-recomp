// addr: 0x8004c304  name: C_011_OBJ_8B0

void C_011_OBJ_8B0(void)

{
  COMMON_DELAY = 0x1325;
  *DAT_800a32d8 = 3;
  DAT_800a32ac = DAT_800a32ac + 1;
  if ((DAT_800a32b8 != 0) && (DAT_800a32a4 != 0)) {
    data_ready_callback();
  }
  return;
}

