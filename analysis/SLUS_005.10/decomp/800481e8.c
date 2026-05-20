// addr: 0x800481e8  name: INTR_OBJ_194

undefined2 * INTR_OBJ_194(void)

{
  undefined2 *puVar1;
  int iVar2;
  
  puVar1 = (undefined2 *)0x0;
  if (DAT_8005eeec == 0) {
    I_MASK = 0;
    I_STAT = 0;
    DMA_DPCR = 0x33333333;
    INTR_OBJ_69C(&DAT_8005eeec,0x41a);
    iVar2 = setjmp((__jmp_buf_tag *)&DAT_8005ef24);
    if (iVar2 != 0) {
      INTR_OBJ_26C();
    }
    DAT_8005ef28 = &DAT_8005ff04;
    HookEntryInt(&DAT_8005ef24);
    DAT_8005eeec = 1;
    DAT_8005ff68 = startIntrVSync();
    DAT_8005ff58 = startIntrDMA();
    FUN_8005398c();
    puVar1 = &DAT_8005eeec;
    FUN_80053a34();
  }
  return puVar1;
}

