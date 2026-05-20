// addr: 0x8010e6b0  name: FUN_8010e6b0

undefined4 FUN_8010e6b0(void)

{
  int iVar1;
  undefined4 local_10;
  
  local_10 = 0x100000;
  if ((*DAT_801124c8 & 0x1000000) != 0) {
    iVar1 = -1;
    do {
      local_10 = local_10 + -1;
      if (local_10 == iVar1) {
        iVar1 = 1;
      }
    } while ((*DAT_801124c8 & 0x1000000) != 0);
  }
  return 0;
}

