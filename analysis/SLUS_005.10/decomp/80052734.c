// addr: 0x80052734  name: PRNT_OBJ_F0

void PRNT_OBJ_F0(uint param_1)

{
  byte bVar1;
  uchar uVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint *puVar7;
  uint *unaff_s1;
  uchar *puVar8;
  char *pcVar9;
  uint unaff_s3;
  int iVar10;
  byte *unaff_s7;
  byte *pbVar11;
  int unaff_s8;
  undefined1 auStackX_0 [16];
  undefined2 uStack00000038;
  uint in_stack_0000003c;
  int in_stack_00000040;
  uint in_stack_00000044;
  char *in_stack_00000048;
  
code_r0x80052734:
  pbVar11 = unaff_s7 + 1;
  bVar1 = *pbVar11;
  if (bVar1 == 0x2a) {
    PRNT_OBJ_168();
    return;
  }
  uVar6 = 0;
  while ((bVar1 < 0x80 && (param_1 = (uint)*pbVar11, ((&DAT_80065175)[param_1] & 4) != 0))) {
    pbVar11 = pbVar11 + 1;
    bVar1 = *pbVar11;
    uVar6 = uVar6 * 10 + -0x30 + param_1;
  }
  unaff_s7 = pbVar11 + -1;
  if ((int)uVar6 < 0) {
    PRNT_OBJ_7C();
    return;
  }
code_r0x800526c0:
  unaff_s7 = unaff_s7 + 1;
  puVar7 = unaff_s1;
  switch(*unaff_s7) {
  case 0:
PRNT_OBJ_630:
    _putchar_flash();
    PRNT_OBJ_664();
    return;
  default:
    uVar6 = (uint)(char)*unaff_s7;
    _uStack00000038 = _uStack00000038 + 1;
    do {
      _putchar(uVar6);
      unaff_s7 = unaff_s7 + 1;
      uVar6 = (uint)*unaff_s7;
      if (uVar6 == 0) goto PRNT_OBJ_630;
      unaff_s3 = 0;
    } while (uVar6 != 0x25);
    uVar6 = 0xffffffff;
    unaff_s8 = 0;
    in_stack_0000003c = 0;
    in_stack_00000040 = 0;
    in_stack_00000044 = 0;
    param_1 = 0x25;
    goto code_r0x800526c0;
  case 0x20:
    if (unaff_s8 == 0) {
      PRNT_OBJ_7C();
      return;
    }
    goto code_r0x800526c0;
  case 0x23:
    unaff_s3 = unaff_s3 | 8;
    goto code_r0x800526c0;
  case 0x2a:
    puVar7 = unaff_s1 + 1;
    in_stack_00000044 = *unaff_s1;
    unaff_s1 = puVar7;
    if (-1 < (int)in_stack_00000044) goto code_r0x800526c0;
    in_stack_00000044 = -in_stack_00000044;
  case 0x2d:
    unaff_s3 = unaff_s3 | 0x10;
    unaff_s1 = puVar7;
    goto code_r0x800526c0;
  case 0x2b:
    unaff_s8 = 0x2b;
    goto code_r0x800526c0;
  case 0x2e:
    goto code_r0x80052734;
  case 0x30:
    unaff_s3 = unaff_s3 | 0x20;
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
    in_stack_00000044 = 0;
    goto PRNT_OBJ_184;
  case 0x44:
    unaff_s3 = unaff_s3 | 1;
  case 100:
  case 0x69:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      PRNT_OBJ_244();
      return;
    }
    uVar4 = *unaff_s1;
    uVar5 = 10;
    if ((int)uVar4 < 0) {
      PRNT_OBJ_3CC();
      return;
    }
    goto code_r0x80052a10;
  case 0x4c:
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
    uVar4 = *unaff_s1;
    uVar5 = 0x10;
    unaff_s8 = 0;
    if (((unaff_s3 & 8) != 0) && (uVar4 != 0)) {
      unaff_s3 = unaff_s3 | 0x40;
      unaff_s8 = 0;
    }
code_r0x80052a10:
    in_stack_0000003c = uVar6;
    if (-1 < (int)uVar6) {
      unaff_s3 = unaff_s3 & 0xffffffdf;
    }
    pcVar9 = (char *)&stack0x00000038;
    if ((uVar4 != 0) || (iVar10 = (int)auStackX_0 - (int)pcVar9, uVar6 != 0)) goto PRNT_OBJ_3F4;
    goto PRNT_OBJ_468;
  case 99:
    PRNT_OBJ_46C();
    return;
  case 0x68:
    unaff_s3 = unaff_s3 | 4;
    goto code_r0x800526c0;
  case 0x6c:
    break;
  case 0x6e:
    if (((unaff_s3 & 1) == 0) && ((unaff_s3 & 4) != 0)) {
      *(undefined2 *)*unaff_s1 = uStack00000038;
      PRNT_OBJ_65C();
      return;
    }
    *(int *)*unaff_s1 = _uStack00000038;
    PRNT_OBJ_65C();
    return;
  case 0x70:
    PRNT_OBJ_3C8(param_1,0x10);
    return;
  case 0x73:
    pcVar9 = (char *)*unaff_s1;
    if ((uchar *)pcVar9 == (uchar *)0x0) {
      pcVar9 = "(null)";
    }
    if ((int)uVar6 < 0) {
      iVar10 = FUN_80052544(pcVar9);
      unaff_s8 = 0;
    }
    else {
      pvVar3 = memchr((uchar *)pcVar9,'\0',uVar6);
      iVar10 = (int)pvVar3 - (int)pcVar9;
      if ((pvVar3 == (void *)0x0) || (unaff_s8 = 0, (int)uVar6 < iVar10)) {
        PRNT_OBJ_334();
        return;
      }
    }
    goto code_r0x80052ab0;
  }
  unaff_s3 = unaff_s3 | 1;
  goto code_r0x800526c0;
  while (((&DAT_80065175)[*unaff_s7] & 4) != 0) {
PRNT_OBJ_184:
    bVar1 = *unaff_s7;
    unaff_s7 = unaff_s7 + 1;
    in_stack_00000044 = in_stack_00000044 * 10 + -0x30 + (uint)bVar1;
    if (0x7f < *unaff_s7) break;
  }
  PRNT_OBJ_7C();
  return;
PRNT_OBJ_3F4:
  do {
    puVar8 = (uchar *)pcVar9;
    uVar6 = uVar4 / uVar5;
    if (uVar5 == 0) {
      trap(0x1c00);
    }
    uVar2 = in_stack_00000048[uVar4 % uVar5];
    pcVar9 = (char *)(puVar8 + -1);
    *pcVar9 = uVar2;
    uVar4 = uVar6;
  } while (uVar6 != 0);
  in_stack_00000048 = "0123456789abcdef";
  if ((unaff_s3 & 8) != 0) {
    iVar10 = (int)auStackX_0 - (int)pcVar9;
    if ((uVar5 != 8) || (iVar10 = (int)auStackX_0 - (int)pcVar9, uVar2 == '0')) goto PRNT_OBJ_468;
    pcVar9 = (char *)(puVar8 + -2);
    *pcVar9 = '0';
  }
  iVar10 = (int)auStackX_0 - (int)pcVar9;
PRNT_OBJ_468:
  iVar10 = iVar10 + 0x38;
code_r0x80052ab0:
  uVar6 = iVar10 + in_stack_00000040;
  if (unaff_s8 != 0) {
    uVar6 = uVar6 + 1;
  }
  if ((unaff_s3 & 0x40) != 0) {
    uVar6 = uVar6 + 2;
  }
  uVar4 = in_stack_0000003c;
  if ((int)in_stack_0000003c < (int)uVar6) {
    uVar4 = uVar6;
  }
  if ((((unaff_s3 & 0x30) == 0) && (in_stack_00000044 != 0)) &&
     (uVar5 = uVar4, (int)uVar4 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar5 = uVar5 + 1;
    } while ((int)uVar5 < (int)in_stack_00000044);
  }
  if (unaff_s8 != 0) {
    _putchar();
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)(char)*unaff_s7);
  }
  if (((unaff_s3 & 0x30) == 0x20) && (uVar5 = uVar4, (int)uVar4 < (int)in_stack_00000044)) {
    do {
      _putchar(0x30);
      uVar5 = uVar5 + 1;
    } while ((int)uVar5 < (int)in_stack_00000044);
  }
  if ((int)uVar6 < (int)in_stack_0000003c) {
    do {
      _putchar(0x30);
      uVar6 = uVar6 + 1;
    } while ((int)uVar6 < (int)in_stack_0000003c);
  }
  while (iVar10 = iVar10 + -1, -1 < iVar10) {
    uVar2 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    _putchar((int)(char)uVar2);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if (((unaff_s3 & 0x10) != 0) && (uVar6 = uVar4, (int)uVar4 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar6 = uVar6 + 1;
    } while ((int)uVar6 < (int)in_stack_00000044);
  }
  uVar6 = in_stack_00000044;
  if ((int)in_stack_00000044 < (int)uVar4) {
    uVar6 = uVar4;
  }
  _uStack00000038 = _uStack00000038 + uVar6;
  PRNT_OBJ_65C();
  return;
}

