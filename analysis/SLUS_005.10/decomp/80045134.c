// addr: 0x80045134  name: FUN_80045134

/* WARNING: Removing unreachable block (ram,0x8004500c) */
/* WARNING: Removing unreachable block (ram,0x8004501c) */
/* WARNING: Removing unreachable block (ram,0x80045038) */
/* WARNING: Removing unreachable block (ram,0x80045040) */
/* WARNING: Removing unreachable block (ram,0x80045058) */
/* WARNING: Removing unreachable block (ram,0x80045078) */

int FUN_80045134(int param_1,int param_2)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  int iVar4;
  
  if (param_1 == 0) {
    return 0;
  }
  if (param_2 != 0) {
    uVar1 = param_2 + 0xfU >> 3;
    iVar4 = *(uint *)(param_1 + -4) - uVar1;
    if (*(uint *)(param_1 + -4) == uVar1) {
      return param_1;
    }
    if (-1 < iVar4) {
      *(uint *)(param_1 + -4) = uVar1;
      *(int *)(param_1 + uVar1 * 8 + -4) = iVar4;
      iVar4 = FUN_80045088();
      return iVar4;
    }
    iVar4 = FUN_80045004(param_2);
    if (iVar4 != 0) {
      FUN_80044d9c(iVar4,param_1,param_2);
      iVar4 = FUN_80045088(param_1);
      return iVar4;
    }
                    /* WARNING: Treating indirect jump as return */
    return 0;
  }
  puVar2 = (uint *)(param_1 + -8);
  puVar3 = DAT_8005ed4c;
  do {
    DAT_8005ed4c = puVar3;
    puVar3 = (uint *)*DAT_8005ed4c;
    if ((DAT_8005ed4c < puVar2) && (puVar2 < puVar3)) break;
  } while ((DAT_8005ed4c < puVar3) || ((puVar2 <= DAT_8005ed4c && (puVar3 <= puVar2))));
  iVar4 = *(int *)(param_1 + -4);
  if ((puVar2 + iVar4 * 2 == puVar3) && (iVar4 = iVar4 + puVar3[1], puVar3[1] != 0)) {
    puVar3 = (uint *)*puVar3;
    *(int *)(param_1 + -4) = iVar4;
  }
  uVar1 = DAT_8005ed4c[1];
  *puVar2 = (uint)puVar3;
  if (DAT_8005ed4c + uVar1 * 2 == puVar2) {
    DAT_8005ed4c[1] = iVar4 + uVar1;
    puVar2 = puVar3;
  }
  *DAT_8005ed4c = (uint)puVar2;
  return 0;
}

