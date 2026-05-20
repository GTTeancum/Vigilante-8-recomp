// addr: 0x80052da4  name: _putchar

void _putchar(byte param_1)

{
  if (param_1 == 9) {
    do {
      _putchar(0x20);
    } while ((DAT_80065204 & 7) != 0);
  }
  else {
    if (param_1 == 10) {
      _putchar(0xd);
      DAT_80065204 = 0;
      PUTCHAR_OBJ_9C();
      return;
    }
    if (((int)(char)(&DAT_80065175)[param_1] & 0x97U) != 0) {
      DAT_80065204 = DAT_80065204 + 1;
    }
    if (0x1f < DAT_80065208) {
      write(1,&DAT_800a4c48,DAT_80065208);
      DAT_80065208 = 0;
    }
    (&DAT_800a4c48)[DAT_80065208] = param_1;
    DAT_80065208 = DAT_80065208 + 1;
  }
  return;
}

