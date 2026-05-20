// addr: 0x80052644  name: prnt

void prnt(undefined4 param_1,byte *param_2,uint *param_3)

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
  undefined1 *puVar10;
  uint *puVar11;
  undefined1 **ppuVar12;
  char *pcVar13;
  uint uVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  undefined1 *puVar17;
  byte *pbVar18;
  int iVar19;
  uint uVar20;
  undefined1 auStack_78 [16];
  undefined1 local_68;
  undefined1 local_42 [2];
  undefined1 *local_40;
  undefined1 *local_3c;
  int local_38;
  undefined1 *local_34;
  char *local_30;
  
  if (param_2 == (byte *)0x0) {
    PRNT_OBJ_664();
    return;
  }
  local_30 = "0123456789abcdef";
  local_40 = (undefined1 *)0x0;
  do {
    uVar8 = (uint)*param_2;
    if (uVar8 == 0) {
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
    if (uVar8 == 0x25) {
      puVar15 = (undefined1 *)0xffffffff;
      iVar19 = 0;
      local_3c = (undefined1 *)0x0;
      local_38 = 0;
      local_34 = (undefined1 *)0x0;
      uVar8 = 0x25;
      pbVar18 = param_2;
code_r0x800526c0:
      param_2 = pbVar18 + 1;
      puVar11 = param_3;
      switch(*param_2) {
      case 0:
        goto PRNT_OBJ_630;
      default:
        uVar8 = (uint)(char)*param_2;
        local_40 = local_40 + 1;
        break;
      case 0x20:
        pbVar18 = param_2;
        if (iVar19 == 0) {
          PRNT_OBJ_7C();
          return;
        }
        goto code_r0x800526c0;
      case 0x23:
        bVar6 = true;
        pbVar18 = param_2;
        goto code_r0x800526c0;
      case 0x2a:
        puVar11 = param_3 + 1;
        local_34 = (undefined1 *)*param_3;
        param_3 = puVar11;
        pbVar18 = param_2;
        if (-1 < (int)local_34) goto code_r0x800526c0;
        local_34 = (undefined1 *)-(int)local_34;
      case 0x2d:
        uVar14 = uVar14 | 0x10;
        param_3 = puVar11;
        pbVar18 = param_2;
        goto code_r0x800526c0;
      case 0x2b:
        iVar19 = 0x2b;
        pbVar18 = param_2;
        goto code_r0x800526c0;
      case 0x2e:
        pbVar18 = pbVar18 + 2;
        bVar1 = *pbVar18;
        if (bVar1 == 0x2a) {
          PRNT_OBJ_168();
          return;
        }
        puVar15 = (undefined1 *)0x0;
        while ((bVar1 < 0x80 && (uVar8 = (uint)*pbVar18, ((&DAT_80065175)[uVar8] & 4) != 0))) {
          pbVar18 = pbVar18 + 1;
          bVar1 = *pbVar18;
          puVar15 = (undefined1 *)((int)puVar15 * 10 + -0x30 + uVar8);
        }
        pbVar18 = pbVar18 + -1;
        if ((int)puVar15 < 0) {
          PRNT_OBJ_7C();
          return;
        }
        goto code_r0x800526c0;
      case 0x30:
        uVar14 = uVar14 | 0x20;
        pbVar18 = param_2;
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
        local_34 = (undefined1 *)0x0;
        goto PRNT_OBJ_184;
      case 0x44:
        bVar4 = true;
      case 100:
      case 0x69:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_244();
          return;
        }
        uVar8 = *param_3;
        uVar9 = 10;
        if ((int)uVar8 < 0) {
          PRNT_OBJ_3CC();
          return;
        }
        goto code_r0x80052a10;
      case 0x4c:
        pbVar18 = param_2;
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
        local_30 = "0123456789ABCDEF";
      case 0x78:
        if ((!bVar4) && (bVar5)) {
          PRNT_OBJ_3B4(uVar8,0x10);
          return;
        }
        uVar8 = *param_3;
        uVar9 = 0x10;
        iVar19 = 0;
        if ((bVar6) && (uVar8 != 0)) {
          bVar3 = true;
          iVar19 = 0;
        }
code_r0x80052a10:
        local_3c = puVar15;
        if (-1 < (int)puVar15) {
          uVar14 = uVar14 & 0xffffffdf;
        }
        pcVar13 = (char *)&local_40;
        if ((uVar8 != 0) || (puVar17 = auStack_78 + -(int)pcVar13, puVar15 != (undefined1 *)0x0))
        goto PRNT_OBJ_3F4;
        goto PRNT_OBJ_468;
      case 99:
        local_68 = (undefined1)*param_3;
        PRNT_OBJ_46C();
        return;
      case 0x68:
        bVar5 = true;
        pbVar18 = param_2;
        goto code_r0x800526c0;
      case 0x6c:
        goto PRNT_OBJ_1EC;
      case 0x6e:
        if ((!bVar4) && (bVar5)) {
          *(undefined2 *)*param_3 = local_40._0_2_;
          PRNT_OBJ_65C();
          return;
        }
        *(undefined1 **)*param_3 = local_40;
        PRNT_OBJ_65C();
        return;
      case 0x70:
        PRNT_OBJ_3C8(uVar8,0x10);
        return;
      case 0x73:
        pcVar13 = (char *)*param_3;
        if ((undefined1 **)pcVar13 == (undefined1 **)0x0) {
          pcVar13 = "(null)";
        }
        if ((int)puVar15 < 0) {
          puVar17 = (undefined1 *)FUN_80052544(pcVar13);
          iVar19 = 0;
        }
        else {
          pvVar7 = memchr((uchar *)pcVar13,'\0',(int)puVar15);
          puVar17 = (undefined1 *)((int)pvVar7 - (int)pcVar13);
          if ((pvVar7 == (void *)0x0) || (iVar19 = 0, (int)puVar15 < (int)puVar17)) {
            PRNT_OBJ_334();
            return;
          }
        }
        goto code_r0x80052ab0;
      }
    }
    _putchar(uVar8);
    param_2 = param_2 + 1;
  } while( true );
PRNT_OBJ_1EC:
  bVar4 = true;
  pbVar18 = param_2;
  goto code_r0x800526c0;
  while (((&DAT_80065175)[*param_2] & 4) != 0) {
PRNT_OBJ_184:
    bVar1 = *param_2;
    param_2 = param_2 + 1;
    local_34 = (undefined1 *)((int)local_34 * 10 + -0x30 + (uint)bVar1);
    if (0x7f < *param_2) break;
  }
  PRNT_OBJ_7C();
  return;
PRNT_OBJ_3F4:
  do {
    ppuVar12 = (undefined1 **)pcVar13;
    uVar20 = uVar8 / uVar9;
    if (uVar9 == 0) {
      trap(0x1c00);
    }
    uVar2 = local_30[uVar8 % uVar9];
    pcVar13 = (char *)((int)ppuVar12 + -1);
    *pcVar13 = uVar2;
    uVar8 = uVar20;
  } while (uVar20 != 0);
  local_30 = "0123456789abcdef";
  if (bVar6) {
    puVar17 = auStack_78 + -(int)pcVar13;
    if ((uVar9 != 8) || (puVar17 = auStack_78 + -(int)pcVar13, uVar2 == '0')) goto PRNT_OBJ_468;
    pcVar13 = (char *)((int)ppuVar12 + -2);
    *pcVar13 = '0';
  }
  puVar17 = auStack_78 + -(int)pcVar13;
PRNT_OBJ_468:
  puVar17 = puVar17 + 0x38;
code_r0x80052ab0:
  puVar15 = puVar17 + local_38;
  if (iVar19 != 0) {
    puVar15 = puVar15 + 1;
  }
  if (bVar3) {
    puVar15 = puVar15 + 2;
  }
  puVar16 = local_3c;
  if ((int)local_3c < (int)puVar15) {
    puVar16 = puVar15;
  }
  if (((uVar14 == 0) && (local_34 != (undefined1 *)0x0)) &&
     (puVar10 = puVar16, (int)puVar16 < (int)local_34)) {
    do {
      _putchar(0x20);
      puVar10 = puVar10 + 1;
    } while ((int)puVar10 < (int)local_34);
  }
  if (iVar19 != 0) {
    _putchar();
  }
  if (bVar3) {
    _putchar(0x30);
    _putchar((int)(char)*param_2);
  }
  if ((uVar14 == 0x20) && (puVar10 = puVar16, (int)puVar16 < (int)local_34)) {
    do {
      _putchar(0x30);
      puVar10 = puVar10 + 1;
    } while ((int)puVar10 < (int)local_34);
  }
  if ((int)puVar15 < (int)local_3c) {
    do {
      _putchar(0x30);
      puVar15 = puVar15 + 1;
    } while ((int)puVar15 < (int)local_3c);
  }
  while (puVar17 = puVar17 + -1, -1 < (int)puVar17) {
    uVar2 = *pcVar13;
    pcVar13 = (char *)((int)pcVar13 + 1);
    _putchar((int)(char)uVar2);
  }
  while (local_38 = local_38 + -1, -1 < local_38) {
    _putchar(0x30);
  }
  if (((uVar14 & 0x10) != 0) && (puVar15 = puVar16, (int)puVar16 < (int)local_34)) {
    do {
      _putchar(0x20);
      puVar15 = puVar15 + 1;
    } while ((int)puVar15 < (int)local_34);
  }
  puVar15 = local_34;
  if ((int)local_34 < (int)puVar16) {
    puVar15 = puVar16;
  }
  local_40 = puVar15 + (int)local_40;
  PRNT_OBJ_65C();
  return;
}

