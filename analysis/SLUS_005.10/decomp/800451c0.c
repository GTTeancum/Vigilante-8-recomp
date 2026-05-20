// addr: 0x800451c0  name: FUN_800451c0

void FUN_800451c0(int param_1,int param_2)

{
  undefined4 *puVar1;
  
  param_2 = param_2 * param_1;
  puVar1 = (undefined4 *)FUN_80045004(param_2);
  if (puVar1 != (undefined4 *)0x0) {
    do {
      *puVar1 = 0;
      puVar1[1] = 0;
      param_2 = param_2 + -8;
      puVar1 = puVar1 + 2;
    } while (0 < param_2);
  }
                    /* WARNING: Treating indirect jump as return */
  return;
}

