// addr: 0x800485d8  name: INTR_OBJ_584

undefined2 * INTR_OBJ_584(void)

{
  undefined2 *puVar1;
  
  puVar1 = (undefined2 *)0x0;
  if (DAT_8005eeec != 0) {
    FUN_80053a24();
    DAT_8005ef1e = I_MASK;
    DAT_8005ef20 = DMA_DPCR;
    I_MASK = 0;
    I_STAT = 0;
    DMA_DPCR = DMA_DPCR & 0x77777777;
    ResetEntryInt();
    DAT_8005eeec = 0;
    puVar1 = &DAT_8005eeec;
  }
  return puVar1;
}

