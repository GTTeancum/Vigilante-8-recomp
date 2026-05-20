// addr: 0x80050ec8  name: SYS_OBJ_1CC4

undefined4 SYS_OBJ_1CC4(short param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_s1;
  dword *unaff_s2;
  
  *(short *)(unaff_s1 + 6) = param_1;
  iVar3 = (int)*(short *)(unaff_s1 + 4) * (int)param_1 + 1;
  iVar4 = iVar3 - (iVar3 >> 0x1f) >> 5;
  if (iVar3 / 2 < 1) {
    uVar1 = SYS_OBJ_1E38();
    return uVar1;
  }
  iVar3 = iVar3 / 2 + iVar4 * -0x10;
  do {
    if ((GPU_REG1 & 0x4000000) != 0) {
      GPU_REG1 = 0x4000000;
      GPU_REG0 = *(dword *)(unaff_s1 + 4);
      while (iVar3 = iVar3 + -1, iVar3 != -1) {
        GPU_REG0 = *unaff_s2;
        unaff_s2 = unaff_s2 + 1;
      }
      if (iVar4 != 0) {
        GPU_REG1 = 0x4000002;
        DMA_GPU_BCR = iVar4 << 0x10 | 0x10;
        DMA_GPU_CHCR = 0x1000201;
        DMA_GPU_MADR = (dword)unaff_s2;
      }
      return 0;
    }
    iVar2 = SYS_OBJ_29C0();
  } while (iVar2 == 0);
  return 0xffffffff;
}

