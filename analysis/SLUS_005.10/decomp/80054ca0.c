// addr: 0x80054ca0  name: _padSetCmd

void _padSetCmd(int param_1,undefined1 param_2,undefined4 param_3,undefined1 param_4)

{
  *(undefined1 *)(param_1 + 0x36) = param_2;
  *(undefined4 *)(param_1 + 0x2c) = param_3;
  *(undefined1 *)(param_1 + 0x35) = param_4;
  return;
}

