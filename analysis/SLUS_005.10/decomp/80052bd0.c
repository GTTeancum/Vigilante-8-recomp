// addr: 0x80052bd0  name: PRNT_OBJ_58C

void PRNT_OBJ_58C(void)

{
  char cVar1;
  int unaff_s0;
  char *unaff_s2;
  uint unaff_s3;
  int unaff_s5;
  int in_stack_00000040;
  int in_stack_00000044;
  
  do {
    cVar1 = *unaff_s2;
    unaff_s2 = unaff_s2 + 1;
    unaff_s0 = unaff_s0 + -1;
    _putchar((int)cVar1);
  } while (-1 < unaff_s0);
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if ((unaff_s3 & 0x10) != 0) {
    for (; unaff_s5 < in_stack_00000044; unaff_s5 = unaff_s5 + 1) {
      _putchar(0x20);
    }
  }
  PRNT_OBJ_65C();
  return;
}

