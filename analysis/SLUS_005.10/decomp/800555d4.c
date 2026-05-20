// addr: 0x800555d4  name: PADCMD_OBJ_940

void PADCMD_OBJ_940(int param_1)

{
  *(undefined1 *)(param_1 + 0x36) = 0x4b;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  *(undefined1 *)(param_1 + 0x35) = 0;
  return;
}

