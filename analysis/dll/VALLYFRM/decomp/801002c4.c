// addr: 0x801002c4  name: FUN_801002c4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801002c4(undefined4 param_1,int param_2)

{
  undefined4 extraout_v1;
  
  if (param_2 != 1) {
    if (param_2 != 7) {
      return 0;
    }
    *(undefined4 *)(_DAT_800659fc + 0x58) = param_1;
    FUN_8001d470/*0x8001d470*/(0x80);
    param_1 = extraout_v1;
  }
  Heap_Free/*0x80045088*/(param_1);
  return 0xffffffff;
}

