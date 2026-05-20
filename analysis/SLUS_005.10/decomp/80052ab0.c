// addr: 0x80052ab0  name: PRNT_OBJ_46C

void PRNT_OBJ_46C(void)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *unaff_s2;
  uint unaff_s3;
  int iVar4;
  int unaff_s6;
  char *unaff_s7;
  int unaff_s8;
  int in_stack_0000003c;
  int in_stack_00000040;
  int in_stack_00000044;
  
  iVar4 = unaff_s6 + in_stack_00000040;
  if (unaff_s8 != 0) {
    iVar4 = iVar4 + 1;
  }
  if ((unaff_s3 & 0x40) != 0) {
    iVar4 = iVar4 + 2;
  }
  iVar3 = in_stack_0000003c;
  if (in_stack_0000003c < iVar4) {
    iVar3 = iVar4;
  }
  if (((unaff_s3 & 0x30) == 0) && (iVar2 = iVar3, in_stack_00000044 != 0)) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x20);
    }
  }
  if (unaff_s8 != 0) {
    _putchar();
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)*unaff_s7);
  }
  iVar2 = iVar3;
  if ((unaff_s3 & 0x30) == 0x20) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x30);
    }
  }
  for (; iVar4 < in_stack_0000003c; iVar4 = iVar4 + 1) {
    _putchar(0x30);
  }
  while (unaff_s6 = unaff_s6 + -1, -1 < unaff_s6) {
    cVar1 = *unaff_s2;
    unaff_s2 = unaff_s2 + 1;
    _putchar((int)cVar1);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if ((unaff_s3 & 0x10) != 0) {
    for (; iVar3 < in_stack_00000044; iVar3 = iVar3 + 1) {
      _putchar(0x20);
    }
  }
  PRNT_OBJ_65C();
  return;
}

