// addr: 0x8004c418  name: C_011_OBJ_9C4

void C_011_OBJ_9C4(void)

{
  int in_v0;
  uint unaff_s0;
  int unaff_s1;
  dword unaff_s2;
  int unaff_s3;
  uint unaff_s4;
  dword in_stack_00000040;
  
  if (unaff_s1 == in_v0) {
    C_011_OBJ_A20(DMA_DICR._2_1_);
    return;
  }
  DMA_DICR._2_1_ = DMA_DICR._2_1_ & ~(byte)(in_v0 << (unaff_s0 & 0x1f));
  DMA_DPCR = DMA_DPCR | 1 << (unaff_s0 * 4 + 3 & 0x1f);
  (&DMA_MDEC_IN_MADR)[unaff_s0 * 4] = unaff_s2;
  (&DMA_MDEC_IN_BCR)[unaff_s0 * 4] = unaff_s3 << 0x10 | unaff_s4;
  do {
  } while ((CDROM_REG0 & 0x40) == 0);
  (&DMA_MDEC_IN_CHCR)[unaff_s0 * 4] = in_stack_00000040;
  return;
}

