// addr: 0x8005667c  name: PADSEQD_OBJ_448

undefined4 PADSEQD_OBJ_448(int param_1)

{
  undefined4 uVar1;
  
  if ((*(short *)(param_1 + 0xe6) == 0) || (uVar1 = 0, *(char *)(param_1 + 0x46) != -1)) {
    uVar1 = 1;
  }
  return uVar1;
}

