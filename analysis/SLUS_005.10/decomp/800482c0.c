// addr: 0x800482c0  name: INTR_OBJ_26C

void INTR_OBJ_26C(void)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  
  if (DAT_8005eeec == 0) {
    printf("unexpected interrupt(%04x)\n",(uint)I_STAT);
    ReturnFromException();
  }
  DAT_8005eeee = 1;
  uVar2 = (uint)(I_MASK & DAT_8005ef1c & I_STAT);
  if (uVar2 != 0) {
    do {
      puVar5 = &DAT_8005eef0;
      for (uVar4 = 0; (uVar2 != 0 && ((int)uVar4 < 0xb)); uVar4 = uVar4 + 1) {
        if ((uVar2 & 1) != 0) {
          I_STAT = ~(ushort)(1 << (uVar4 & 0x1f));
          if ((code *)*puVar5 != (code *)0x0) {
            (*(code *)*puVar5)();
          }
        }
        puVar5 = puVar5 + 1;
        uVar2 = uVar2 >> 1;
      }
      uVar2 = (uint)(I_MASK & DAT_8005ef1c & I_STAT);
    } while (uVar2 != 0);
  }
  if ((I_STAT & I_MASK) == 0) {
    DAT_8005ff84 = 0;
  }
  else {
    iVar3 = DAT_8005ff84 + 1;
    bVar1 = 0x800 < DAT_8005ff84;
    DAT_8005ff84 = iVar3;
    if (bVar1) {
      printf("intr timeout(%04x:%04x)\n",(uint)I_STAT,(uint)I_MASK);
      DAT_8005ff84 = 0;
      I_STAT = 0;
      INTR_OBJ_410();
      return;
    }
  }
  DAT_8005eeee = 0;
  ReturnFromException();
  return;
}

