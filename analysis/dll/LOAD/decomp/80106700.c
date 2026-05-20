// addr: 0x80106700  name: FUN_80106700

undefined4 FUN_80106700(void)

{
  int iVar1;
  undefined4 local_10;
  
  local_10 = 0x100000;
  if ((*DAT_80106f40 & 0x20000000) != 0) {
    iVar1 = -1;
    do {
      local_10 = local_10 + -1;
      if (local_10 == iVar1) {
        iVar1 = 1;
      }
    } while ((*DAT_80106f40 & 0x20000000) != 0);
  }
  return 0;
}

