// addr: 0x80100e98  name: FUN_80100e98

void FUN_80100e98(int param_1)

{
  undefined4 uVar1;
  int local_res0 [4];
  uint local_18 [2];
  
  local_res0[0] = param_1;
  while (local_res0[0] != 0) {
    uVar1 = Iff_ReadChunkData/*0x800225d4*/(local_18,local_res0);
    Heap_Free/*0x80045088*/(uVar1);
  }
  return;
}

