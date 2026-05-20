// addr: 0x80045088  name: FUN_80045088

void FUN_80045088(int param_1)

{
  int iVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  
  puVar2 = (uint *)(param_1 + -8);
  puVar3 = DAT_8005ed4c;
  do {
    DAT_8005ed4c = puVar3;
    puVar3 = (uint *)*DAT_8005ed4c;
    if ((DAT_8005ed4c < puVar2) && (puVar2 < puVar3)) break;
  } while ((DAT_8005ed4c < puVar3) || ((puVar2 <= DAT_8005ed4c && (puVar3 <= puVar2))));
  iVar1 = *(int *)(param_1 + -4);
  if ((puVar2 + iVar1 * 2 == puVar3) && (iVar1 = iVar1 + puVar3[1], puVar3[1] != 0)) {
    puVar3 = (uint *)*puVar3;
    *(int *)(param_1 + -4) = iVar1;
  }
  uVar4 = DAT_8005ed4c[1];
  *puVar2 = (uint)puVar3;
  if (DAT_8005ed4c + uVar4 * 2 == puVar2) {
    DAT_8005ed4c[1] = iVar1 + uVar4;
    puVar2 = puVar3;
  }
  *DAT_8005ed4c = (uint)puVar2;
  return;
}

