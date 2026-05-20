// addr: 0x80052004  name: SYS_OBJ_2E00

undefined4 SYS_OBJ_2E00(void)

{
  int iVar1;
  uint in_v0;
  undefined4 uVar2;
  uint in_v1;
  undefined4 *unaff_s0;
  int unaff_s1;
  uint unaff_s2;
  
  while (((in_v0 & in_v1) != 0 || ((GPU_REG1 & 0x4000000) == 0))) {
    iVar1 = SYS_OBJ_29C0();
    if (iVar1 != 0) {
      return 0xffffffff;
    }
    in_v1 = 0x1000000;
    in_v0 = DMA_GPU_CHCR;
  }
  DMACallback(2,SYS_OBJ_2FBC);
  uVar2 = 0xffffffff;
  if (*(short *)(unaff_s0 + 1) != 0) {
    if (*(short *)((int)unaff_s0 + 6) == 0) {
      uVar2 = SYS_OBJ_2EA8();
      return uVar2;
    }
    DAT_800650c8 = unaff_s1 << 0x10 | unaff_s2 & 0xffff;
    DAT_800650c4 = *unaff_s0;
    DAT_800650cc = unaff_s0[1];
    SYS_OBJ_2154(&DAT_800650bc);
    uVar2 = 0;
  }
  return uVar2;
}

