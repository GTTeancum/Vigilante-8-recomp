// addr: 0x800529b8  name: PRNT_OBJ_374

/* WARNING: Removing unreachable block (ram,0x80052abc) */
/* WARNING: Removing unreachable block (ram,0x80052a44) */
/* WARNING: Removing unreachable block (ram,0x80052a8c) */
/* WARNING: Removing unreachable block (ram,0x80052aa0) */
/* WARNING: Removing unreachable block (ram,0x80052b2c) */

void PRNT_OBJ_374(undefined4 param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint *unaff_s1;
  char *pcVar5;
  uint unaff_s3;
  int unaff_s4;
  int iVar6;
  int iVar7;
  char *unaff_s7;
  uint uVar8;
  undefined1 auStackX_0 [16];
  int in_stack_00000038;
  int in_stack_00000040;
  int in_stack_00000044;
  char *pcStack00000048;
  
  pcStack00000048 = "0123456789ABCDEF";
  if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
    PRNT_OBJ_3B4(param_1,0x10);
    return;
  }
  uVar3 = *unaff_s1;
  if (((unaff_s3 & 8) != 0) && (uVar3 != 0)) {
    unaff_s3 = unaff_s3 | 0x40;
  }
  if (-1 < unaff_s4) {
    unaff_s3 = unaff_s3 & 0xffffffdf;
  }
  pcVar5 = (char *)&stack0x00000038;
  if ((uVar3 != 0) || (iVar4 = (int)auStackX_0 - (int)pcVar5, unaff_s4 != 0)) {
    do {
      uVar8 = uVar3 / 0x10;
      pcVar5 = pcVar5 + -1;
      *pcVar5 = "0123456789ABCDEF"[uVar3 % 0x10];
      uVar3 = uVar8;
    } while (uVar8 != 0);
    pcStack00000048 = "0123456789abcdef";
    if ((unaff_s3 & 8) == 0) {
      iVar4 = (int)auStackX_0 - (int)pcVar5;
    }
    else {
      iVar4 = (int)auStackX_0 - (int)pcVar5;
    }
  }
  iVar6 = iVar4 + 0x38 + in_stack_00000040;
  if ((unaff_s3 & 0x40) != 0) {
    iVar6 = iVar6 + 2;
  }
  iVar7 = unaff_s4;
  if (unaff_s4 < iVar6) {
    iVar7 = iVar6;
  }
  if (((unaff_s3 & 0x30) == 0) && (iVar2 = iVar7, in_stack_00000044 != 0)) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x20);
    }
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)*unaff_s7);
  }
  iVar2 = iVar7;
  if ((unaff_s3 & 0x30) == 0x20) {
    for (; iVar2 < in_stack_00000044; iVar2 = iVar2 + 1) {
      _putchar(0x30);
    }
  }
  for (; iVar6 < unaff_s4; iVar6 = iVar6 + 1) {
    _putchar(0x30);
  }
  for (iVar4 = iVar4 + 0x37; -1 < iVar4; iVar4 = iVar4 + -1) {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    _putchar((int)cVar1);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  iVar4 = iVar7;
  if ((unaff_s3 & 0x10) != 0) {
    for (; iVar4 < in_stack_00000044; iVar4 = iVar4 + 1) {
      _putchar(0x20);
    }
  }
  if (in_stack_00000044 < iVar7) {
    in_stack_00000044 = iVar7;
  }
  in_stack_00000038 = in_stack_00000038 + in_stack_00000044;
  PRNT_OBJ_65C();
  return;
}

