// addr: 0x8004c39c  name: C_011_OBJ_948

void C_011_OBJ_948(uint param_1,dword param_2,int param_3,uint param_4,dword param_5,char param_6)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = 0;
  uVar1 = (&DMA_MDEC_IN_CHCR)[param_1 * 4];
  while ((uVar1 & 0x1000000) != 0) {
    if (iVar2 == 0x10000) {
      printf("DMA STATUS ERROR %x\n",(&DMA_MDEC_IN_CHCR)[param_1 * 4]);
      C_011_OBJ_9C4();
      return;
    }
    iVar2 = iVar2 + 1;
    uVar1 = (&DMA_MDEC_IN_CHCR)[param_1 * 4];
  }
  if (param_6 == '\x01') {
    C_011_OBJ_A20(DMA_DICR._2_1_);
    return;
  }
  DMA_DICR._2_1_ = DMA_DICR._2_1_ & ~(byte)(1 << (param_1 & 0x1f));
  DMA_DPCR = DMA_DPCR | 1 << (param_1 * 4 + 3 & 0x1f);
  (&DMA_MDEC_IN_MADR)[param_1 * 4] = param_2;
  (&DMA_MDEC_IN_BCR)[param_1 * 4] = param_3 << 0x10 | param_4;
  do {
  } while ((CDROM_REG0 & 0x40) == 0);
  (&DMA_MDEC_IN_CHCR)[param_1 * 4] = param_5;
  return;
}

