// addr: 0x80106794  name: FUN_80106794

undefined4 FUN_80106794(void)

{
  int iVar1;
  undefined4 local_10;
  
  local_10 = 0x100000;
  if ((*DAT_80106f20 & 0x1000000) != 0) {
    iVar1 = -1;
    do {
      local_10 = local_10 + -1;
      if (local_10 == iVar1) {
        iVar1 = 1;
      }
    } while ((*DAT_80106f20 & 0x1000000) != 0);
  }
  return 0;
}

