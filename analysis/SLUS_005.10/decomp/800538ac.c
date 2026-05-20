// addr: 0x800538ac  name: PUTS_OBJ_28

void PUTS_OBJ_28(void)

{
  char cVar1;
  char *unaff_s0;
  
  while( true ) {
    cVar1 = *unaff_s0;
    unaff_s0 = unaff_s0 + 1;
    if (cVar1 == '\0') break;
    _putchar((int)cVar1);
  }
  _putchar_flash();
  return;
}

