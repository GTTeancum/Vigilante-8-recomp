// addr: 0x8003fc50  name: FUN_8003fc50

bool FUN_8003fc50(int param_1)

{
  uint uVar1;
  
  uVar1 = FUN_8003fbc8();
  if (uVar1 != 0) {
    FUN_8003fac4(param_1,*(undefined4 *)(param_1 + 0x58),uVar1 & 0xffff);
  }
  return uVar1 != 0;
}

