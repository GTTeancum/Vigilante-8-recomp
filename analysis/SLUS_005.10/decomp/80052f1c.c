// addr: 0x80052f1c  name: PUTCHAR_OBJ_178

void PUTCHAR_OBJ_178(void)

{
  do {
    _putchar(0x20);
  } while ((DAT_80065204 & 7) != 0);
  if (0 < DAT_80065208) {
    write(1,&DAT_800a4c48,DAT_80065208);
    DAT_80065208 = 0;
  }
  return;
}

