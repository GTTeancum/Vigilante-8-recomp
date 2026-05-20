// addr: 0x80052e40  name: PUTCHAR_OBJ_9C

void PUTCHAR_OBJ_9C(void)

{
  undefined1 unaff_s0;
  
  if (0x1f < DAT_80065208) {
    write(1,&DAT_800a4c48,DAT_80065208);
    DAT_80065208 = 0;
  }
  (&DAT_800a4c48)[DAT_80065208] = unaff_s0;
  DAT_80065208 = DAT_80065208 + 1;
  return;
}

