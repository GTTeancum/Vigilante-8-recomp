// addr: 0x800555b4  name: PADCMD_OBJ_920

void PADCMD_OBJ_920(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x36) = 0x47;
  *(int *)(param_1 + 0x2c) = param_1 + 0x24;
  *(undefined1 *)(param_1 + 0x24) = param_2;
  *(undefined1 *)(param_1 + 0x35) = 1;
  return;
}

