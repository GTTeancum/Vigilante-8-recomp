// addr: 0x80022e38  name: FUN_80022e38

void FUN_80022e38(int param_1)

{
  code *pcVar1;
  
  if (0x3c < iRam0000000c) {
    *(undefined1 *)(param_1 + 8) = 1;
  }
  *(undefined2 *)(param_1 + 0xa4) = 0;
  *(undefined2 *)(param_1 + 0xa6) = 0x3c;
  pcVar1 = *(code **)(*(int *)(param_1 + 0x10c) + 100);
  if (pcVar1 != (code *)0x0) {
    (*pcVar1)(*(int *)(param_1 + 0x10c),0xb);
  }
  return;
}

