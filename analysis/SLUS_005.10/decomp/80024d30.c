// addr: 0x80024d30  name: FUN_80024d30

/* WARNING: This function may have set the stack pointer */

void FUN_80024d30(void)

{
  undefined4 unaff_retaddr;
  undefined1 auStackX_0 [16];
  
  DAT_1f8003f0 = unaff_retaddr;
  DAT_1f8003f4 = (undefined1 *)register0x00000074;
  FUN_80024d54();
  return;
}

