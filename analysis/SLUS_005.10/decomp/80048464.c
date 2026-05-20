// addr: 0x80048464  name: INTR_OBJ_410

void INTR_OBJ_410(void)

{
  DAT_8005eeee = 0;
  ReturnFromException();
  return;
}

