// addr: 0x80048880  name: INTR_DMA_OBJ_4C

void INTR_DMA_OBJ_4C(void)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  uVar3 = DMA_DICR >> 0x18 & 0x7f;
  if (uVar3 != 0) {
    do {
      puVar4 = &DAT_8005ffc8;
      for (iVar1 = 0; (uVar3 != 0 && (iVar1 < 7)); iVar1 = iVar1 + 1) {
        if ((uVar3 & 1) != 0) {
          DMA_DICR = DMA_DICR & (1 << (iVar1 + 0x18U & 0x1f) | 0xffffffU);
          if ((code *)*puVar4 != (code *)0x0) {
            (*(code *)*puVar4)();
          }
        }
        puVar4 = puVar4 + 1;
        uVar3 = uVar3 >> 1;
      }
      uVar3 = DMA_DICR >> 0x18 & 0x7f;
    } while (uVar3 != 0);
  }
  if (((DMA_DICR & 0xff000000) == 0x80000000) || ((DMA_DICR & 0x8000) != 0)) {
    printf("DMA bus error: code=%08x\n",DMA_DICR);
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 1;
      printf("MADR[%d]=%08x\n",iVar1,(&DMA_MDEC_IN_MADR)[iVar1 * 4]);
      iVar1 = iVar2;
    } while (iVar2 < 7);
  }
  return;
}

