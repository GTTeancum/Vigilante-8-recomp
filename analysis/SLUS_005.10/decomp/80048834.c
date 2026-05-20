// addr: 0x80048834  name: startIntrDMA

code * startIntrDMA(void)

{
  INTR_DMA_OBJ_278(&DAT_8005ffc8,8);
  DMA_DICR = 0;
  InterruptCallback(3,INTR_DMA_OBJ_4C);
  return INTR_DMA_OBJ_1CC;
}

