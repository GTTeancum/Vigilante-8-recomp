// addr: 0x80051a20  name: SYS_OBJ_281C

undefined4 SYS_OBJ_281C(void)

{
  undefined4 uVar1;
  uint unaff_s0;
  
  SetIntrMask(DAT_8006515c);
  uVar1 = 0;
  if ((unaff_s0 & 7) == 0) {
    uVar1 = SYS_OBJ_2B04();
  }
  return uVar1;
}

