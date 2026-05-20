// addr: 0x80048678  name: INTR_OBJ_624

undefined4 INTR_OBJ_624(void)

{
  undefined4 uVar1;
  
  if (DAT_8005eeec == 0) {
    HookEntryInt(&DAT_8005ef24);
    DAT_8005eeec = 1;
    I_MASK = DAT_8005ef1e;
    DMA_DPCR = DAT_8005ef20;
    FUN_80053a34();
    uVar1 = INTR_OBJ_68C();
    return uVar1;
  }
  return 0;
}

