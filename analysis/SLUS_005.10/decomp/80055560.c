// addr: 0x80055560  name: PADCMD_OBJ_8CC

void PADCMD_OBJ_8CC(int param_1)

{
  *(undefined1 *)(param_1 + 0x36) = 0x45;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  *(undefined1 *)(param_1 + 0x35) = 0;
  return;
}

