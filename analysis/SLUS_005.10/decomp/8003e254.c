// addr: 0x8003e254  name: FUN_8003e254

undefined4 * FUN_8003e254(undefined4 param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)FUN_800116f4(0x2c);
  *puVar1 = param_1;
  if (param_2 < 0) {
    param_2 = param_2 + 0xf;
  }
  puVar1[9] = param_2 >> 4;
  if (param_3 < 0) {
    param_3 = param_3 + 0xf;
  }
  puVar1[10] = param_3 >> 4;
  return puVar1;
}

