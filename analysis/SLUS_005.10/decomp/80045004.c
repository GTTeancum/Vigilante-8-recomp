// addr: 0x80045004  name: FUN_80045004

undefined4 * FUN_80045004(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  if (param_1 != 0) {
    uVar3 = param_1 + 0xfU >> 3;
    puVar4 = DAT_8005ed4c;
    do {
      puVar2 = (undefined4 *)*puVar4;
      iVar1 = puVar2[1] - uVar3;
      if (-1 < iVar1) {
        if (iVar1 == 0) {
          DAT_8005ed4c = puVar4;
          *puVar4 = *puVar2;
          return puVar2 + 2;
        }
        puVar2[1] = iVar1;
        puVar2[iVar1 * 2 + 1] = uVar3;
        DAT_8005ed4c = puVar4;
        return puVar2 + iVar1 * 2 + 2;
      }
      puVar4 = puVar2;
    } while (puVar2 != DAT_8005ed4c);
  }
  return (undefined4 *)0x0;
}

