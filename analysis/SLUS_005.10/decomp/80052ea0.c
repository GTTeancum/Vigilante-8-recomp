// addr: 0x80052ea0  name: _putchar_flash

void _putchar_flash(void)

{
  if (0 < DAT_80065208) {
    write(1,&DAT_800a4c48,DAT_80065208);
    DAT_80065208 = 0;
  }
  return;
}

