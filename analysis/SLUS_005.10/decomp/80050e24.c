// addr: 0x80050e24  name: SYS_OBJ_1C20

/* WARNING: Removing unreachable block (ram,0x80050f90) */

undefined4 SYS_OBJ_1C20(int param_1,dword *param_2)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined2 uVar4;
  short sVar5;
  int iVar6;
  
  SYS_OBJ_298C();
  uVar4 = *(undefined2 *)(param_1 + 4);
  if (*(short *)(param_1 + 4) < 0) {
    uVar4 = 0;
  }
  else if (DAT_80065028 < *(short *)(param_1 + 4)) {
    uVar1 = SYS_OBJ_1C88(DAT_80065028);
    return uVar1;
  }
  sVar5 = *(short *)(param_1 + 6);
  *(undefined2 *)(param_1 + 4) = uVar4;
  if (*(short *)(param_1 + 6) < 0) {
    sVar5 = 0;
  }
  else if (DAT_8006502a < *(short *)(param_1 + 6)) {
    uVar1 = SYS_OBJ_1CC4(DAT_8006502a);
    return uVar1;
  }
  *(short *)(param_1 + 6) = sVar5;
  iVar3 = (int)*(short *)(param_1 + 4) * (int)sVar5 + 1;
  iVar6 = iVar3 - (iVar3 >> 0x1f) >> 5;
  if (0 < iVar3 / 2) {
    iVar3 = iVar3 / 2 + iVar6 * -0x10;
    do {
      if ((GPU_REG1 & 0x4000000) != 0) {
        GPU_REG1 = 0x4000000;
        GPU_REG0 = *(dword *)(param_1 + 4);
        while (iVar3 = iVar3 + -1, iVar3 != -1) {
          GPU_REG0 = *param_2;
          param_2 = param_2 + 1;
        }
        if (iVar6 != 0) {
          GPU_REG1 = 0x4000002;
          DMA_GPU_BCR = iVar6 << 0x10 | 0x10;
          DMA_GPU_CHCR = 0x1000201;
          DMA_GPU_MADR = (dword)param_2;
        }
        return 0;
      }
      iVar2 = SYS_OBJ_29C0();
    } while (iVar2 == 0);
    return 0xffffffff;
  }
  uVar1 = SYS_OBJ_1E38();
  return uVar1;
}

