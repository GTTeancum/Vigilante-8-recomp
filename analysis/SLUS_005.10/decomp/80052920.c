// addr: 0x80052920  name: PRNT_OBJ_2DC

/* WARNING: Removing unreachable block (ram,0x80052abc) */
/* WARNING: Removing unreachable block (ram,0x80052b2c) */

void PRNT_OBJ_2DC(void)

{
  uchar uVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  int *unaff_s1;
  char *pcVar5;
  uint unaff_s3;
  int unaff_s4;
  int iVar6;
  int iVar7;
  char *unaff_s7;
  int in_stack_0000003c;
  int in_stack_00000040;
  int in_stack_00000044;
  
  pcVar5 = (char *)*unaff_s1;
  if ((uchar *)pcVar5 == (uchar *)0x0) {
    pcVar5 = "(null)";
  }
  if (unaff_s4 < 0) {
    iVar7 = FUN_80052544(pcVar5);
  }
  else {
    pvVar3 = memchr((uchar *)pcVar5,'\0',unaff_s4);
    iVar7 = (int)pvVar3 - (int)pcVar5;
    if ((pvVar3 == (void *)0x0) || (unaff_s4 < iVar7)) {
      PRNT_OBJ_334();
      return;
    }
  }
  iVar6 = iVar7 + in_stack_00000040;
  if ((unaff_s3 & 0x40) != 0) {
    iVar6 = iVar6 + 2;
  }
  iVar4 = in_stack_0000003c;
  if (in_stack_0000003c < iVar6) {
    iVar4 = iVar6;
  }
  if (((unaff_s3 & 0x30) == 0) && (iVar2 = iVar4, in_stack_00000044 != 0)) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x20);
    }
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)*unaff_s7);
  }
  iVar2 = iVar4;
  if ((unaff_s3 & 0x30) == 0x20) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x30);
    }
  }
  for (; iVar6 < in_stack_0000003c; iVar6 = iVar6 + 1) {
    _putchar(0x30);
  }
  while (iVar7 = iVar7 + -1, -1 < iVar7) {
    uVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    _putchar((int)(char)uVar1);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if ((unaff_s3 & 0x10) != 0) {
    for (; iVar4 < in_stack_00000044; iVar4 = iVar4 + 1) {
      _putchar(0x20);
    }
  }
  PRNT_OBJ_65C();
  return;
}

