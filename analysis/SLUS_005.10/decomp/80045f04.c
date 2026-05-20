// addr: 0x80045f04  name: _spu_FsetPCR

void _spu_FsetPCR(int param_1)

{
  DMA_DPCR = DMA_DPCR & 0xfff8ffff;
  if (param_1 != 0) {
    SPU_OBJ_A7C(0x30000);
    return;
  }
  DMA_DPCR = DMA_DPCR | 0x50000;
  return;
}

