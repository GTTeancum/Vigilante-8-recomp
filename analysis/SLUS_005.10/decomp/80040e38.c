// addr: 0x80040e38  name: FUN_80040e38

/* WARNING: This function may have set the stack pointer */

void FUN_80040e38(void)

{
  undefined4 unaff_retaddr;
  undefined1 auStackX_0 [16];
  
  DAT_1f8003f0 = unaff_retaddr;
  DAT_1f8003f4 = (undefined1 *)register0x00000074;
  FUN_80040e5c();
  return;
}

