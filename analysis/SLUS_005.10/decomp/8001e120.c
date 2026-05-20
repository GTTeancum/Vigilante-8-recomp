// addr: 0x8001e120  name: FUN_8001e120

undefined4 FUN_8001e120(int param_1)

{
  undefined4 uVar1;
  undefined4 unaff_retaddr;
  undefined1 auStackX_0 [16];
  
  if (*(code **)(param_1 + 100) != (code *)0x0) {
    DAT_1f8003f8 = unaff_retaddr;
    DAT_1f8003fc = (undefined1 *)register0x00000074;
    uVar1 = (**(code **)(param_1 + 100))();
    return uVar1;
  }
  return 0;
}

