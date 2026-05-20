// addr: 0x80051b90  name: SYS_OBJ_298C

void SYS_OBJ_298C(void)

{
  int iVar1;
  
  iVar1 = VSync(-1);
  DAT_80065160 = iVar1 + 0xf0;
  DAT_80065164 = 0;
  return;
}

