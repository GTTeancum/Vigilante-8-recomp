// addr: 0x80052820  name: PRNT_OBJ_1DC

void PRNT_OBJ_1DC(uint param_1)

{
  byte bVar1;
  uchar uVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint *unaff_s1;
  uchar *puVar7;
  char *pcVar8;
  uint unaff_s3;
  uint unaff_s4;
  int iVar9;
  byte *pbVar10;
  byte *unaff_s7;
  int unaff_s8;
  uint uVar11;
  undefined1 auStackX_0 [16];
  undefined2 uStack00000038;
  uint in_stack_0000003c;
  int in_stack_00000040;
  uint in_stack_00000044;
  char *in_stack_00000048;
  
code_r0x80052820:
  pbVar10 = unaff_s7;
code_r0x800526c0:
  unaff_s7 = pbVar10 + 1;
  puVar6 = unaff_s1;
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
    unaff_s4 = 0xffffffff;
    unaff_s8 = 0;
    in_stack_0000003c = 0;
    in_stack_00000040 = 0;
    in_stack_00000044 = 0;
    param_1 = 0x25;
    pbVar10 = unaff_s7;
    goto code_r0x800526c0;
  case 0x20:
    pbVar10 = unaff_s7;
    if (unaff_s8 == 0) {
      PRNT_OBJ_7C();
      return;
    }
    goto code_r0x800526c0;
  case 0x23:
    unaff_s3 = unaff_s3 | 8;
    pbVar10 = unaff_s7;
    goto code_r0x800526c0;
  case 0x2a:
    puVar6 = unaff_s1 + 1;
    in_stack_00000044 = *unaff_s1;
    unaff_s1 = puVar6;
    pbVar10 = unaff_s7;
    if (-1 < (int)in_stack_00000044) goto code_r0x800526c0;
    in_stack_00000044 = -in_stack_00000044;
  case 0x2d:
    unaff_s3 = unaff_s3 | 0x10;
    unaff_s1 = puVar6;
    pbVar10 = unaff_s7;
    goto code_r0x800526c0;
  case 0x2b:
    unaff_s8 = 0x2b;
    pbVar10 = unaff_s7;
    goto code_r0x800526c0;
  case 0x2e:
    pbVar10 = pbVar10 + 2;
    bVar1 = *pbVar10;
    if (bVar1 == 0x2a) {
      PRNT_OBJ_168();
      return;
    }
    unaff_s4 = 0;
    while ((bVar1 < 0x80 && (param_1 = (uint)*pbVar10, ((&DAT_80065175)[param_1] & 4) != 0))) {
      pbVar10 = pbVar10 + 1;
      bVar1 = *pbVar10;
      unaff_s4 = unaff_s4 * 10 + -0x30 + param_1;
    }
    pbVar10 = pbVar10 + -1;
    if ((int)unaff_s4 < 0) {
      PRNT_OBJ_7C();
      return;
    }
    goto code_r0x800526c0;
  case 0x30:
    unaff_s3 = unaff_s3 | 0x20;
    pbVar10 = unaff_s7;
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
    goto code_r0x80052820;
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
    in_stack_0000003c = unaff_s4;
    if (-1 < (int)unaff_s4) {
      unaff_s3 = unaff_s3 & 0xffffffdf;
    }
    pcVar8 = (char *)&stack0x00000038;
    if ((uVar4 != 0) || (iVar9 = (int)auStackX_0 - (int)pcVar8, unaff_s4 != 0)) goto PRNT_OBJ_3F4;
    goto PRNT_OBJ_468;
  case 99:
    PRNT_OBJ_46C();
    return;
  case 0x68:
    unaff_s3 = unaff_s3 | 4;
    pbVar10 = unaff_s7;
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
    pcVar8 = (char *)*unaff_s1;
    if ((uchar *)pcVar8 == (uchar *)0x0) {
      pcVar8 = "(null)";
    }
    if ((int)unaff_s4 < 0) {
      iVar9 = FUN_80052544(pcVar8);
      unaff_s8 = 0;
    }
    else {
      pvVar3 = memchr((uchar *)pcVar8,'\0',unaff_s4);
      iVar9 = (int)pvVar3 - (int)pcVar8;
      if ((pvVar3 == (void *)0x0) || (unaff_s8 = 0, (int)unaff_s4 < iVar9)) {
        PRNT_OBJ_334();
        return;
      }
    }
    goto code_r0x80052ab0;
  }
  unaff_s3 = unaff_s3 | 1;
  pbVar10 = unaff_s7;
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
    puVar7 = (uchar *)pcVar8;
    uVar11 = uVar4 / uVar5;
    if (uVar5 == 0) {
      trap(0x1c00);
    }
    uVar2 = in_stack_00000048[uVar4 % uVar5];
    pcVar8 = (char *)(puVar7 + -1);
    *pcVar8 = uVar2;
    uVar4 = uVar11;
  } while (uVar11 != 0);
  in_stack_00000048 = "0123456789abcdef";
  if ((unaff_s3 & 8) != 0) {
    iVar9 = (int)auStackX_0 - (int)pcVar8;
    if ((uVar5 != 8) || (iVar9 = (int)auStackX_0 - (int)pcVar8, uVar2 == '0')) goto PRNT_OBJ_468;
    pcVar8 = (char *)(puVar7 + -2);
    *pcVar8 = '0';
  }
  iVar9 = (int)auStackX_0 - (int)pcVar8;
PRNT_OBJ_468:
  iVar9 = iVar9 + 0x38;
code_r0x80052ab0:
  uVar4 = iVar9 + in_stack_00000040;
  if (unaff_s8 != 0) {
    uVar4 = uVar4 + 1;
  }
  if ((unaff_s3 & 0x40) != 0) {
    uVar4 = uVar4 + 2;
  }
  uVar5 = in_stack_0000003c;
  if ((int)in_stack_0000003c < (int)uVar4) {
    uVar5 = uVar4;
  }
  if ((((unaff_s3 & 0x30) == 0) && (in_stack_00000044 != 0)) &&
     (uVar11 = uVar5, (int)uVar5 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar11 = uVar11 + 1;
    } while ((int)uVar11 < (int)in_stack_00000044);
  }
  if (unaff_s8 != 0) {
    _putchar();
  }
  if ((unaff_s3 & 0x40) != 0) {
    _putchar(0x30);
    _putchar((int)(char)*unaff_s7);
  }
  if (((unaff_s3 & 0x30) == 0x20) && (uVar11 = uVar5, (int)uVar5 < (int)in_stack_00000044)) {
    do {
      _putchar(0x30);
      uVar11 = uVar11 + 1;
    } while ((int)uVar11 < (int)in_stack_00000044);
  }
  if ((int)uVar4 < (int)in_stack_0000003c) {
    do {
      _putchar(0x30);
      uVar4 = uVar4 + 1;
    } while ((int)uVar4 < (int)in_stack_0000003c);
  }
  while (iVar9 = iVar9 + -1, -1 < iVar9) {
    uVar2 = *pcVar8;
    pcVar8 = pcVar8 + 1;
    _putchar((int)(char)uVar2);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if (((unaff_s3 & 0x10) != 0) && (uVar4 = uVar5, (int)uVar5 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar4 = uVar4 + 1;
    } while ((int)uVar4 < (int)in_stack_00000044);
  }
  uVar4 = in_stack_00000044;
  if ((int)in_stack_00000044 < (int)uVar5) {
    uVar4 = uVar5;
  }
  _uStack00000038 = _uStack00000038 + uVar4;
  PRNT_OBJ_65C();
  return;
}

