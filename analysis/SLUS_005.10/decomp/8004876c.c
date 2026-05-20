// addr: 0x8004876c  name: INTR_VB_OBJ_58

void INTR_VB_OBJ_58(void)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = 0;
  puVar1 = &DAT_8005ff94;
  DAT_8005ffb4 = DAT_8005ffb4 + 1;
  do {
    if ((code *)*puVar1 != (code *)0x0) {
      (*(code *)*puVar1)();
    }
    iVar2 = iVar2 + 1;
    puVar1 = puVar1 + 1;
  } while (iVar2 < 8);
  return;
}

