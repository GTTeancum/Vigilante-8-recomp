// addr: 0x800502dc  name: GetODE

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int GetODE(void)

{
  uint uVar1;
  
  uVar1 = SYS_OBJ_18F8();
  return uVar1 >> 0x1f;
}

