// addr: 0x80052888  name: PRNT_OBJ_244

/* WARNING: Removing unreachable block (ram,0x80052a44) */
/* WARNING: Removing unreachable block (ram,0x80052a8c) */
/* WARNING: Removing unreachable block (ram,0x80052aa0) */

void PRNT_OBJ_244(void)

{
  char cVar1;
  int iVar2;
  uint in_v1;
  int iVar3;
  char *pcVar4;
  uint unaff_s3;
  int unaff_s4;
  int iVar5;
  int iVar6;
  char *unaff_s7;
  int unaff_s8;
  uint uVar7;
  undefined1 auStackX_0 [16];
  int in_stack_00000038;
  int in_stack_00000040;
  int in_stack_00000044;
  int in_stack_00000048;
  
  if ((int)in_v1 < 0) {
    PRNT_OBJ_3CC();
    return;
  }
  if (-1 < unaff_s4) {
    unaff_s3 = unaff_s3 & 0xffffffdf;
  }
  pcVar4 = (char *)&stack0x00000038;
  if ((in_v1 != 0) || (iVar3 = (int)auStackX_0 - (int)pcVar4, unaff_s4 != 0)) {
    do {
      uVar7 = in_v1 / 10;
      pcVar4 = pcVar4 + -1;
      *pcVar4 = *(char *)(in_stack_00000048 + in_v1 % 10);
      in_v1 = uVar7;
    } while (uVar7 != 0);
    if ((unaff_s3 & 8) == 0) {
      iVar3 = (int)auStackX_0 - (int)pcVar4;
    }
    else {
      iVar3 = (int)auStackX_0 - (int)pcVar4;
    }
  }
  iVar5 = iVar3 + 0x38 + in_stack_00000040;
  if (unaff_s8 != 0) {
    iVar5 = iVar5 + 1;
  }
  if ((unaff_s3 & 0x40) != 0) {
    iVar5 = iVar5 + 2;
  }
  iVar6 = unaff_s4;
  if (unaff_s4 < iVar5) {
    iVar6 = iVar5;
  }
  if (((unaff_s3 & 0x30) == 0) && (iVar2 = iVar6, in_stack_00000044 != 0)) {
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
  iVar2 = iVar6;
  if ((unaff_s3 & 0x30) == 0x20) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x30);
    }
  }
  for (; iVar5 < unaff_s4; iVar5 = iVar5 + 1) {
    _putchar(0x30);
  }
  for (iVar3 = iVar3 + 0x37; -1 < iVar3; iVar3 = iVar3 + -1) {
    cVar1 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    _putchar((int)cVar1);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  iVar3 = iVar6;
  if ((unaff_s3 & 0x10) != 0) {
    for (; iVar3 < in_stack_00000044; iVar3 = iVar3 + 1) {
      _putchar(0x20);
    }
  }
  if (in_stack_00000044 < iVar6) {
    in_stack_00000044 = iVar6;
  }
  in_stack_00000038 = in_stack_00000038 + in_stack_00000044;
  PRNT_OBJ_65C();
  return;
}

