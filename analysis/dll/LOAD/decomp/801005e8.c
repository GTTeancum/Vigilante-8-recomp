// addr: 0x801005e8  name: FUN_801005e8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801005e8(undefined4 param_1,undefined4 param_2)

{
  _DAT_800659f0 = Heap_AllocOrRetry/*0x800116f4*/(param_2);
  V8_MemCopy/*0x80044c44*/(_DAT_800659f0,param_1,param_2);
  _DAT_800659e8 = param_2;
  return;
}

