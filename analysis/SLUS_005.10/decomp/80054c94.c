// addr: 0x80054c94  name: _padSetAct

void _padSetAct(int param_1,undefined4 param_2,undefined1 param_3)

{
  *(undefined4 *)(param_1 + 0x28) = param_2;
  *(undefined1 *)(param_1 + 0x34) = param_3;
  return;
}

