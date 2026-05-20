// addr: 0x80052708  name: PRNT_OBJ_C4

void PRNT_OBJ_C4(uint param_1)

{
  byte bVar1;
  uchar uVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int *unaff_s1;
  int *piVar7;
  uchar *puVar8;
  char *pcVar9;
  uint unaff_s3;
  int unaff_s4;
  int iVar10;
  int iVar11;
  int iVar12;
  byte *unaff_s7;
  byte *pbVar13;
  int unaff_s8;
  uint uVar14;
  undefined1 auStackX_0 [16];
  undefined2 uStack00000038;
  int in_stack_0000003c;
  int in_stack_00000040;
  int iStack00000044;
  char *in_stack_00000048;
  
code_r0x80052708:
  piVar7 = unaff_s1;
  iStack00000044 = *piVar7;
  pbVar13 = unaff_s7;
  if (-1 < iStack00000044) goto code_r0x800526c0;
  iStack00000044 = -iStack00000044;
PRNT_OBJ_E0:
  unaff_s3 = unaff_s3 | 0x10;
  pbVar13 = unaff_s7;
code_r0x800526c0:
  unaff_s7 = pbVar13 + 1;
  unaff_s1 = piVar7 + 1;
  switch(*unaff_s7) {
  case 0:
PRNT_OBJ_630:
    _putchar_flash();
    PRNT_OBJ_664();
    return;
  default:
    uVar4 = (uint)(char)*unaff_s7;
    _uStack00000038 = _uStack00000038 + 1;
    do {
      _putchar(uVar4);
      unaff_s7 = unaff_s7 + 1;
      uVar4 = (uint)*unaff_s7;
      if (uVar4 == 0) goto PRNT_OBJ_630;
      unaff_s3 = 0;
    } while (uVar4 != 0x25);
    unaff_s4 = -1;
    unaff_s8 = 0;
    in_stack_0000003c = 0;
    in_stack_00000040 = 0;
    iStack00000044 = 0;
    param_1 = 0x25;
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x20:
    pbVar13 = unaff_s7;
    if (unaff_s8 == 0) {
      PRNT_OBJ_7C();
      return;
    }
    goto code_r0x800526c0;
  case 0x23:
    unaff_s3 = unaff_s3 | 8;
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x2a:
    goto code_r0x80052708;
  case 0x2b:
    unaff_s8 = 0x2b;
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x2d:
    goto PRNT_OBJ_E0;
  case 0x2e:
    pbVar13 = pbVar13 + 2;
    bVar1 = *pbVar13;
    if (bVar1 == 0x2a) {
      PRNT_OBJ_168();
      return;
    }
    unaff_s4 = 0;
    while ((bVar1 < 0x80 && (param_1 = (uint)*pbVar13, ((&DAT_80065175)[param_1] & 4) != 0))) {
      pbVar13 = pbVar13 + 1;
      bVar1 = *pbVar13;
      unaff_s4 = unaff_s4 * 10 + -0x30 + param_1;
    }
    pbVar13 = pbVar13 + -1;
    if (unaff_s4 < 0) {
      PRNT_OBJ_7C();
      return;
    }
    goto code_r0x800526c0;
  case 0x30:
    unaff_s3 = unaff_s3 | 0x20;
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x39:
    iStack00000044 = 0;
    goto PRNT_OBJ_184;
  case 0x44:
    unaff_s3 = unaff_s3 | 1;
  case 100:
  case 0x69:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      PRNT_OBJ_244();
      return;
    }
    uVar4 = piVar7[1];
    uVar5 = 10;
    if ((int)uVar4 < 0) {
      PRNT_OBJ_3CC();
      return;
    }
    break;
  case 0x4c:
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x4f:
    unaff_s3 = unaff_s3 | 1;
  case 0x6f:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      PRNT_OBJ_3CC(param_1,8);
      return;
    }
    PRNT_OBJ_3C8(param_1,8);
    return;
  case 0x55:
    unaff_s3 = unaff_s3 | 1;
  case 0x75:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      PRNT_OBJ_3CC(param_1,10);
      return;
    }
    PRNT_OBJ_3C8(param_1,10);
    return;
  case 0x58:
    in_stack_00000048 = "0123456789ABCDEF";
  case 0x78:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      PRNT_OBJ_3B4(param_1,0x10);
      return;
    }
    uVar4 = piVar7[1];
    uVar5 = 0x10;
    unaff_s8 = 0;
    if (((unaff_s3 & 8) != 0) && (uVar4 != 0)) {
      unaff_s3 = unaff_s3 | 0x40;
      unaff_s8 = 0;
    }
    break;
  case 99:
    PRNT_OBJ_46C();
    return;
  case 0x68:
    unaff_s3 = unaff_s3 | 4;
    pbVar13 = unaff_s7;
    goto code_r0x800526c0;
  case 0x6c:
    goto PRNT_OBJ_1EC;
  case 0x6e:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      *(undefined2 *)piVar7[1] = uStack00000038;
      PRNT_OBJ_65C();
      return;
    }
    *(int *)piVar7[1] = _uStack00000038;
    PRNT_OBJ_65C();
    return;
  case 0x70:
    PRNT_OBJ_3C8(param_1,0x10);
    return;
  case 0x73:
    pcVar9 = (char *)piVar7[1];
    if ((uchar *)pcVar9 == (uchar *)0x0) {
      pcVar9 = "(null)";
    }
    if (unaff_s4 < 0) {
      iVar12 = FUN_80052544(pcVar9);
      unaff_s8 = 0;
    }
    else {
      pvVar3 = memchr((uchar *)pcVar9,'\0',unaff_s4);
      iVar12 = (int)pvVar3 - (int)pcVar9;
      if ((pvVar3 == (void *)0x0) || (unaff_s8 = 0, unaff_s4 < iVar12)) {
        PRNT_OBJ_334();
        return;
      }
    }
    goto code_r0x80052ab0;
  }
  in_stack_0000003c = unaff_s4;
  if (-1 < unaff_s4) {
    unaff_s3 = unaff_s3 & 0xffffffdf;
  }
  pcVar9 = (char *)&stack0x00000038;
  if ((uVar4 != 0) || (iVar12 = (int)auStackX_0 - (int)pcVar9, unaff_s4 != 0)) goto PRNT_OBJ_3F4;
  goto PRNT_OBJ_468;
PRNT_OBJ_1EC:
  unaff_s3 = unaff_s3 | 1;
  pbVar13 = unaff_s7;
  goto code_r0x800526c0;
  while (((&DAT_80065175)[*unaff_s7] & 4) != 0) {
PRNT_OBJ_184:
    bVar1 = *unaff_s7;
    unaff_s7 = unaff_s7 + 1;
    iStack00000044 = iStack00000044 * 10 + -0x30 + (uint)bVar1;
    if (0x7f < *unaff_s7) break;
  }
  PRNT_OBJ_7C();
  return;
PRNT_OBJ_3F4:
  do {
    puVar8 = (uchar *)pcVar9;
    uVar14 = uVar4 / uVar5;
    if (uVar5 == 0) {
      trap(0x1c00);
    }
    uVar2 = in_stack_00000048[uVar4 % uVar5];
    pcVar9 = (char *)(puVar8 + -1);
    *pcVar9 = uVar2;
    uVar4 = uVar14;
  } while (uVar14 != 0);
  in_stack_00000048 = "0123456789abcdef";
  if ((unaff_s3 & 8) != 0) {
    iVar12 = (int)auStackX_0 - (int)pcVar9;
    if ((uVar5 != 8) || (iVar12 = (int)auStackX_0 - (int)pcVar9, uVar2 == '0')) goto PRNT_OBJ_468;
    pcVar9 = (char *)(puVar8 + -2);
    *pcVar9 = '0';
  }
  iVar12 = (int)auStackX_0 - (int)pcVar9;
PRNT_OBJ_468:
  iVar12 = iVar12 + 0x38;
code_r0x80052ab0:
  iVar10 = iVar12 + in_stack_00000040;
  if (unaff_s8 != 0) {
    iVar10 = iVar10 + 1;
  }
  if ((unaff_s3 & 0x40) != 0) {
    iVar10 = iVar10 + 2;
  }
  iVar11 = in_stack_0000003c;
  if (in_stack_0000003c < iVar10) {
    iVar11 = iVar10;
  }
  if ((((unaff_s3 & 0x30) == 0) && (iStack00000044 != 0)) &&
     (iVar6 = iVar11, iVar11 < iStack00000044)) {
    do {
      _putchar(0x20);
      iVar6 = iVar6 + 1;
    } while (iVar6 < iStack00000044);
  }
  if (unaff_s8 != 0) {
    _putchar();
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)(char)*unaff_s7);
  }
  if (((unaff_s3 & 0x30) == 0x20) && (iVar6 = iVar11, iVar11 < iStack00000044)) {
    do {
      _putchar(0x30);
      iVar6 = iVar6 + 1;
    } while (iVar6 < iStack00000044);
  }
  if (iVar10 < in_stack_0000003c) {
    do {
      _putchar(0x30);
      iVar10 = iVar10 + 1;
    } while (iVar10 < in_stack_0000003c);
  }
  while (iVar12 = iVar12 + -1, -1 < iVar12) {
    uVar2 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    _putchar((int)(char)uVar2);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if (((unaff_s3 & 0x10) != 0) && (iVar12 = iVar11, iVar11 < iStack00000044)) {
    do {
      _putchar(0x20);
      iVar12 = iVar12 + 1;
    } while (iVar12 < iStack00000044);
  }
  iVar12 = iStack00000044;
  if (iStack00000044 < iVar11) {
    iVar12 = iVar11;
  }
  _uStack00000038 = _uStack00000038 + iVar12;
  PRNT_OBJ_65C();
  return;
}

