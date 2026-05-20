// addr: 0x80048a00  name: INTR_DMA_OBJ_1CC

int INTR_DMA_OBJ_1CC(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = &DAT_8005ffc8 + param_1;
  iVar2 = *piVar1;
  if (param_2 != iVar2) {
    if (param_2 != 0) {
      *piVar1 = param_2;
      DMA_DICR = DMA_DICR & 0xffffff | 1 << (param_1 + 0x10U & 0x1f) | 0x800000U;
      iVar2 = INTR_DMA_OBJ_270();
      return iVar2;
    }
    *piVar1 = 0;
    DMA_DICR = (DMA_DICR & 0xffffff | 0x800000) & ~(1 << (param_1 + 0x10U & 0x1f));
  }
  return iVar2;
}

