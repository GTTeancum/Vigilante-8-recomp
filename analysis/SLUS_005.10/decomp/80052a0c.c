// addr: 0x80052a0c  name: PRNT_OBJ_3C8

/* WARNING: Removing unreachable block (ram,0x80052abc) */
/* WARNING: Removing unreachable block (ram,0x80052b2c) */

void PRNT_OBJ_3C8(undefined4 param_1,uint param_2)

{
  char cVar1;
  int iVar2;
  uint in_v1;
  int iVar3;
  char *pcVar4;
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
  int in_stack_00000048;
  
  if (-1 < unaff_s4) {
    unaff_s3 = unaff_s3 & 0xffffffdf;
  }
  pcVar5 = (char *)&stack0x00000038;
  if ((in_v1 != 0) || (iVar3 = (int)auStackX_0 - (int)pcVar5, unaff_s4 != 0)) {
    do {
      pcVar4 = pcVar5;
      uVar8 = in_v1 / param_2;
      if (param_2 == 0) {
        trap(0x1c00);
      }
      cVar1 = *(char *)(in_stack_00000048 + in_v1 % param_2);
      pcVar5 = pcVar4 + -1;
      *pcVar5 = cVar1;
      in_v1 = uVar8;
    } while (uVar8 != 0);
    if ((unaff_s3 & 8) != 0) {
      iVar3 = (int)auStackX_0 - (int)pcVar5;
      if ((param_2 != 8) || (iVar3 = (int)auStackX_0 - (int)pcVar5, cVar1 == '0'))
      goto PRNT_OBJ_468;
      pcVar5 = pcVar4 + -2;
      *pcVar5 = '0';
    }
    iVar3 = (int)auStackX_0 - (int)pcVar5;
  }
PRNT_OBJ_468:
  iVar6 = iVar3 + 0x38 + in_stack_00000040;
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
  for (iVar3 = iVar3 + 0x37; -1 < iVar3; iVar3 = iVar3 + -1) {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    _putchar((int)cVar1);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  iVar3 = iVar7;
  if ((unaff_s3 & 0x10) != 0) {
    for (; iVar3 < in_stack_00000044; iVar3 = iVar3 + 1) {
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

