// addr: 0x80052ca0  name: PRNT_OBJ_65C

void PRNT_OBJ_65C(void)

{
  byte bVar1;
  uchar uVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  void *pvVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint *puVar11;
  uint *unaff_s1;
  uchar *puVar12;
  char *pcVar13;
  uint uVar14;
  int iVar15;
  byte *pbVar16;
  byte *unaff_s7;
  int iVar17;
  undefined1 auStackX_0 [16];
  undefined2 uStack00000038;
  uint in_stack_0000003c;
  int in_stack_00000040;
  uint in_stack_00000044;
  char *in_stack_00000048;
  
  do {
    unaff_s7 = unaff_s7 + 1;
    uVar9 = (uint)*unaff_s7;
    if (uVar9 == 0) {
PRNT_OBJ_630:
      _putchar_flash();
      PRNT_OBJ_664();
      return;
    }
    uVar14 = 0;
    bVar3 = false;
    bVar6 = false;
    bVar5 = false;
    bVar4 = false;
    if (uVar9 == 0x25) {
      uVar9 = 0xffffffff;
      iVar17 = 0;
      in_stack_0000003c = 0;
      in_stack_00000040 = 0;
      in_stack_00000044 = 0;
      uVar8 = 0x25;
      pbVar16 = unaff_s7;
code_r0x800526c0:
      unaff_s7 = pbVar16 + 1;
      puVar11 = unaff_s1;
      switch(*unaff_s7) {
      case 0:
        goto PRNT_OBJ_630;
      default:
        uVar9 = (uint)(char)*unaff_s7;
        _uStack00000038 = _uStack00000038 + 1;
        break;
      case 0x20:
        pbVar16 = unaff_s7;
        if (iVar17 == 0) {
          PRNT_OBJ_7C();
          return;
        }
        goto code_r0x800526c0;
      case 0x23:
        bVar6 = true;
        pbVar16 = unaff_s7;
        goto code_r0x800526c0;
      case 0x2a:
        puVar11 = unaff_s1 + 1;
        in_stack_00000044 = *unaff_s1;
        unaff_s1 = puVar11;
        pbVar16 = unaff_s7;
        if (-1 < (int)in_stack_00000044) goto code_r0x800526c0;
        in_stack_00000044 = -in_stack_00000044;
      case 0x2d:
        uVar14 = uVar14 | 0x10;
        unaff_s1 = puVar11;
        pbVar16 = unaff_s7;
        goto code_r0x800526c0;
      case 0x2b:
        iVar17 = 0x2b;
        pbVar16 = unaff_s7;
        goto code_r0x800526c0;
      case 0x2e:
        pbVar16 = pbVar16 + 2;
        bVar1 = *pbVar16;
        if (bVar1 == 0x2a) {
          PRNT_OBJ_168();
          return;
        }
        uVar9 = 0;
        while ((bVar1 < 0x80 && (uVar8 = (uint)*pbVar16, ((&DAT_80065175)[uVar8] & 4) != 0))) {
          pbVar16 = pbVar16 + 1;
          bVar1 = *pbVar16;
          uVar9 = uVar9 * 10 + -0x30 + uVar8;
        }
        pbVar16 = pbVar16 + -1;
        if ((int)uVar9 < 0) {
          PRNT_OBJ_7C();
          return;
        }
        goto code_r0x800526c0;
      case 0x30:
        uVar14 = uVar14 | 0x20;
        pbVar16 = unaff_s7;
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
        bVar4 = true;
      case 100:
      case 0x69:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_244();
          return;
        }
        uVar8 = *unaff_s1;
        uVar10 = 10;
        if ((int)uVar8 < 0) {
          PRNT_OBJ_3CC();
          return;
        }
        goto code_r0x80052a10;
      case 0x4c:
        pbVar16 = unaff_s7;
        goto code_r0x800526c0;
      case 0x4f:
        bVar4 = true;
      case 0x6f:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_3CC(uVar8,8);
          return;
        }
        PRNT_OBJ_3C8(uVar8,8);
        return;
      case 0x55:
        bVar4 = true;
      case 0x75:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_3CC(uVar8,10);
          return;
        }
        PRNT_OBJ_3C8(uVar8,10);
        return;
      case 0x58:
        in_stack_00000048 = "0123456789ABCDEF";
      case 0x78:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_3B4(uVar8,0x10);
          return;
        }
        uVar8 = *unaff_s1;
        uVar10 = 0x10;
        iVar17 = 0;
        if ((bVar6) && (uVar8 != 0)) {
          bVar3 = true;
          iVar17 = 0;
        }
code_r0x80052a10:
        in_stack_0000003c = uVar9;
        if (-1 < (int)uVar9) {
          uVar14 = uVar14 & 0xffffffdf;
        }
        pcVar13 = (char *)&stack0x00000038;
        if ((uVar8 != 0) || (iVar15 = (int)auStackX_0 - (int)pcVar13, uVar9 != 0))
        goto PRNT_OBJ_3F4;
        goto PRNT_OBJ_468;
      case 99:
        PRNT_OBJ_46C();
        return;
      case 0x68:
        bVar5 = true;
        pbVar16 = unaff_s7;
        goto code_r0x800526c0;
      case 0x6c:
        goto PRNT_OBJ_1EC;
      case 0x6e:
        if ((!bVar4) && (bVar5)) {
          *(undefined2 *)*unaff_s1 = uStack00000038;
          PRNT_OBJ_65C();
          return;
        }
        *(int *)*unaff_s1 = _uStack00000038;
        PRNT_OBJ_65C();
        return;
      case 0x70:
        PRNT_OBJ_3C8(uVar8,0x10);
        return;
      case 0x73:
        pcVar13 = (char *)*unaff_s1;
        if ((uchar *)pcVar13 == (uchar *)0x0) {
          pcVar13 = "(null)";
        }
        if ((int)uVar9 < 0) {
          iVar15 = FUN_80052544(pcVar13);
          iVar17 = 0;
        }
        else {
          pvVar7 = memchr((uchar *)pcVar13,'\0',uVar9);
          iVar15 = (int)pvVar7 - (int)pcVar13;
          if ((pvVar7 == (void *)0x0) || (iVar17 = 0, (int)uVar9 < iVar15)) {
            PRNT_OBJ_334();
            return;
          }
        }
        goto code_r0x80052ab0;
      }
    }
    _putchar(uVar9);
  } while( true );
PRNT_OBJ_1EC:
  bVar4 = true;
  pbVar16 = unaff_s7;
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
    puVar12 = (uchar *)pcVar13;
    uVar9 = uVar8 / uVar10;
    if (uVar10 == 0) {
      trap(0x1c00);
    }
    uVar2 = in_stack_00000048[uVar8 % uVar10];
    pcVar13 = (char *)(puVar12 + -1);
    *pcVar13 = uVar2;
    uVar8 = uVar9;
  } while (uVar9 != 0);
  in_stack_00000048 = "0123456789abcdef";
  if (bVar6) {
    iVar15 = (int)auStackX_0 - (int)pcVar13;
    if ((uVar10 != 8) || (iVar15 = (int)auStackX_0 - (int)pcVar13, uVar2 == '0')) goto PRNT_OBJ_468;
    pcVar13 = (char *)(puVar12 + -2);
    *pcVar13 = '0';
  }
  iVar15 = (int)auStackX_0 - (int)pcVar13;
PRNT_OBJ_468:
  iVar15 = iVar15 + 0x38;
code_r0x80052ab0:
  uVar9 = iVar15 + in_stack_00000040;
  if (iVar17 != 0) {
    uVar9 = uVar9 + 1;
  }
  if (bVar3) {
    uVar9 = uVar9 + 2;
  }
  uVar8 = in_stack_0000003c;
  if ((int)in_stack_0000003c < (int)uVar9) {
    uVar8 = uVar9;
  }
  if (((uVar14 == 0) && (in_stack_00000044 != 0)) &&
     (uVar10 = uVar8, (int)uVar8 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar10 = uVar10 + 1;
    } while ((int)uVar10 < (int)in_stack_00000044);
  }
  if (iVar17 != 0) {
    _putchar();
  }
  if (bVar3) {
    _putchar(0x30);
    _putchar((int)(char)*unaff_s7);
  }
  if ((uVar14 == 0x20) && (uVar10 = uVar8, (int)uVar8 < (int)in_stack_00000044)) {
    do {
      _putchar(0x30);
      uVar10 = uVar10 + 1;
    } while ((int)uVar10 < (int)in_stack_00000044);
  }
  if ((int)uVar9 < (int)in_stack_0000003c) {
    do {
      _putchar(0x30);
      uVar9 = uVar9 + 1;
    } while ((int)uVar9 < (int)in_stack_0000003c);
  }
  while (iVar15 = iVar15 + -1, -1 < iVar15) {
    uVar2 = *pcVar13;
    pcVar13 = pcVar13 + 1;
    _putchar((int)(char)uVar2);
  }
  while (in_stack_00000040 = in_stack_00000040 + -1, -1 < in_stack_00000040) {
    _putchar(0x30);
  }
  if (((uVar14 & 0x10) != 0) && (uVar9 = uVar8, (int)uVar8 < (int)in_stack_00000044)) {
    do {
      _putchar(0x20);
      uVar9 = uVar9 + 1;
    } while ((int)uVar9 < (int)in_stack_00000044);
  }
  uVar9 = in_stack_00000044;
  if ((int)in_stack_00000044 < (int)uVar8) {
    uVar9 = uVar8;
  }
  _uStack00000038 = _uStack00000038 + uVar9;
  PRNT_OBJ_65C();
  return;
}

