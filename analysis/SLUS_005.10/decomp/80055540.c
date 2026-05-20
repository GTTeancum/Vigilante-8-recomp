// addr: 0x80055540  name: _padCmdParaMode

void _padCmdParaMode(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x36) = 0x43;
  *(int *)(param_1 + 0x2c) = param_1 + 0x24;
  *(undefined1 *)(param_1 + 0x24) = param_2;
  *(undefined1 *)(param_1 + 0x35) = 1;
  return;
}

