// addr: 0x8002c958  name: FUN_8002c958

undefined4 FUN_8002c958(int param_1)

{
  undefined4 uVar1;
  
  if (*(short *)(param_1 + 0x11e) == 0) {
    uVar1 = FUN_8002c6fc();
  }
  else {
    FUN_8002c018();
    uVar1 = 0;
  }
  return uVar1;
}

