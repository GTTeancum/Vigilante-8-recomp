// addr: 0x80016b08  name: FUN_80016b08

undefined2 * FUN_80016b08(int *param_1,undefined2 *param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  
  iVar1 = FUN_80016a20();
  if (iVar1 == 0) {
    param_2[2] = 0;
    param_2[1] = 0;
    *param_2 = 0;
  }
  else {
    gte_ldLZCS(iVar1);
    iVar2 = gte_stLZCR();
    uVar3 = 0xc;
    if ((int)(iVar2 - 1U) < 0xc) {
      uVar3 = iVar2 - 1U;
    }
    iVar1 = iVar1 >> (0xc - uVar3 & 0x1f);
    *param_2 = (short)((*param_1 << (uVar3 & 0x1f)) / iVar1);
    param_2[1] = (short)((param_1[1] << (uVar3 & 0x1f)) / iVar1);
    param_2[2] = (short)((param_1[2] << (uVar3 & 0x1f)) / iVar1);
  }
  return param_2;
}

