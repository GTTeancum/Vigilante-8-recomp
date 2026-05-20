// addr: 0x8004b9f4  name: init_ring_status

void init_ring_status(int param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = 0;
  if (param_2 != 0) {
    do {
      iVar1 = uVar2 + param_1;
      uVar2 = uVar2 + 1;
      *(undefined4 *)(DAT_800a32c8 + iVar1 * 0x20) = 0;
    } while (uVar2 < param_2);
  }
  return;
}

