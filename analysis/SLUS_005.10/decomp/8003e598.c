// addr: 0x8003e598  name: FUN_8003e598

void FUN_8003e598(int param_1,undefined4 param_2)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  short *psVar4;
  short *psVar5;
  bool bVar6;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  
  psVar4 = *(short **)(param_1 + 0x5c);
  bVar6 = true;
  if (psVar4 != (short *)0x0) {
    sVar1 = *psVar4;
    iVar2 = local_c;
joined_r0x8003e5c4:
    if (sVar1 != 0) {
      sVar1 = *psVar4;
      if (sVar1 != 1) goto code_r0x8003e5e0;
      if (bVar6) {
        local_20 = *(int *)(psVar4 + 2);
        local_1c = *(int *)(psVar4 + 4);
        local_18 = *(int *)(psVar4 + 6);
        local_14 = *(int *)(psVar4 + 8);
        local_10 = *(int *)(psVar4 + 10);
        local_c = *(int *)(psVar4 + 0xc);
        bVar6 = false;
LAB_8003e6d8:
        psVar5 = psVar4 + 0xe;
        iVar2 = local_c;
      }
      else {
        if (*(int *)(psVar4 + 2) < local_20) {
          local_20 = *(int *)(psVar4 + 2);
        }
        if (*(int *)(psVar4 + 4) < local_1c) {
          local_1c = *(int *)(psVar4 + 4);
        }
        if (*(int *)(psVar4 + 6) < local_18) {
          local_18 = *(int *)(psVar4 + 6);
        }
        if (local_14 < *(int *)(psVar4 + 8)) {
          local_14 = *(int *)(psVar4 + 8);
        }
        if (local_10 < *(int *)(psVar4 + 10)) {
          local_10 = *(int *)(psVar4 + 10);
        }
        local_c = *(int *)(psVar4 + 0xc);
        psVar5 = psVar4 + 0xe;
        if (iVar2 < local_c) goto LAB_8003e6d8;
      }
      goto LAB_8003e6fc;
    }
    uVar3 = FUN_8003e254(param_2,local_14,iVar2);
    *(undefined4 *)(param_1 + 0x70) = uVar3;
  }
  return;
code_r0x8003e5e0:
  if (sVar1 == 2) {
    psVar5 = psVar4 + (uint)(ushort)psVar4[1] * 6 + 2;
LAB_8003e6fc:
    local_c = iVar2;
    sVar1 = *psVar5;
    psVar4 = psVar5;
    iVar2 = local_c;
  }
  goto joined_r0x8003e5c4;
}

