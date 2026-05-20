// addr: 0x80053884  name: puts

void puts(char *param_1)

{
  char cVar1;
  
  if (param_1 == (char *)0x0) {
    PUTS_OBJ_28();
    return;
  }
  while( true ) {
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    if (cVar1 == '\0') break;
    _putchar((int)cVar1);
  }
  _putchar_flash();
  return;
}

