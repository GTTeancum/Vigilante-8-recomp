// addr: 0x8001ac08  name: FUN_8001ac08

void FUN_8001ac08(int param_1)

{
  undefined2 uVar1;
  undefined4 uVar2;
  
  uVar2 = FUN_8001ab98(*(undefined4 *)(param_1 + 0x58),*(undefined2 *)(param_1 + 10));
  uVar1 = uRam000006cc;
  *(undefined4 *)(param_1 + 0x60) = uVar2;
  *(undefined2 *)(param_1 + 0x46) = uVar1;
  return;
}

