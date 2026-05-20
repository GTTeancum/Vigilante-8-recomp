// addr: 0x80053004  name: sprintf

/* WARNING: Removing unreachable block (ram,0x80053334) */
/* WARNING: Removing unreachable block (ram,0x8005338c) */
/* WARNING: Removing unreachable block (ram,0x800535d4) */
/* WARNING: Removing unreachable block (ram,0x800534ac) */
/* WARNING: Removing unreachable block (ram,0x8005377c) */

int sprintf(char *buffer,char *fmt,...)

{
  char cVar1;
  char *pcVar2;
  void *pvVar3;
  uint uVar4;
  char *pcVar5;
  int iVar6;
  uint in_a2;
  undefined4 in_a3;
  uint uVar7;
  byte *pbVar8;
  int *piVar9;
  int iVar10;
  char *local_res4;
  uint local_res8;
  undefined4 local_resc;
  byte local_39;
  undefined4 local_38;
  uint local_34;
  uint local_30;
  uint *local_28;
  
  local_28 = &local_res8;
  iVar10 = 0;
  cVar1 = *fmt;
  local_res8 = in_a2;
  local_resc = in_a3;
  while (iVar6 = (int)cVar1, iVar6 != 0) {
    if (iVar6 != 0x25) {
SPRINTF_OBJ_794:
      buffer[iVar10] = (char)iVar6;
      iVar10 = SPRINTF_OBJ_82C();
      return iVar10;
    }
    local_38 = 0;
    local_34 = 0;
    local_30 = 0;
    local_res4 = fmt;
    while( true ) {
      while( true ) {
        while( true ) {
          while( true ) {
            while( true ) {
              pcVar2 = local_res4 + 1;
              iVar6 = (int)local_res4[1];
              if (iVar6 != 0x2d) break;
              local_38 = local_38 | 1;
              local_res4 = pcVar2;
            }
            if (iVar6 != 0x2b) break;
            local_38 = local_38 | 2;
            local_res4 = pcVar2;
          }
          if (iVar6 != 0x20) break;
          local_38._0_2_ = CONCAT11(local_res4[1],(byte)local_38);
          local_38 = (uint)(ushort)local_38;
          local_res4 = pcVar2;
        }
        if (iVar6 != 0x23) break;
        local_38 = local_38 | 4;
        local_res4 = pcVar2;
      }
      if (iVar6 != 0x30) break;
      local_38 = local_38 | 8;
      local_res4 = pcVar2;
    }
    if (iVar6 == 0x2a) {
      uVar4 = *local_28;
      local_28 = local_28 + 1;
      local_34 = uVar4;
      if ((int)uVar4 < 0) {
        local_34 = -uVar4;
        local_38 = local_38 | 1;
      }
      iVar10 = SPRINTF_OBJ_1BC(uVar4,(int)local_res4[2]);
      return iVar10;
    }
    while (local_res4 = pcVar2, pcVar2 = local_res4, iVar6 - 0x30U < 10) {
      local_34 = local_34 * 10 + -0x30 + iVar6;
      pcVar2 = local_res4 + 1;
      iVar6 = (int)local_res4[1];
    }
    pcVar5 = (char *)0x23;
    if (iVar6 == 0x2e) {
      iVar6 = (int)local_res4[1];
      pcVar5 = local_res4;
      if (iVar6 == 0x2a) {
        local_30 = *local_28;
        local_28 = local_28 + 1;
        iVar10 = SPRINTF_OBJ_258(local_res4,(int)local_res4[2]);
        return iVar10;
      }
      while (local_res4 = pcVar5 + 1, iVar6 - 0x30U < 10) {
        local_30 = local_30 * 10 + -0x30 + iVar6;
        iVar6 = (int)pcVar5[2];
        pcVar5 = local_res4;
      }
      pcVar5 = pcVar2;
      if (-1 < (int)local_30) {
        local_38 = local_38 | 0x10;
      }
    }
    pbVar8 = (byte *)&local_38;
    if ((local_38 & 1) != 0) {
      local_38 = local_38 & 0xfffffff7;
    }
    switch(iVar6) {
    case 0x4c:
      local_38 = local_38 | 0x80;
      iVar10 = SPRINTF_OBJ_29C(pcVar5,(int)local_res4[1]);
      return iVar10;
    default:
      if (iVar6 == 0x25) goto SPRINTF_OBJ_794;
      goto SPRINTF_OBJ_84C;
    case 99:
      local_28 = local_28 + 1;
      iVar10 = SPRINTF_OBJ_7A0();
      return iVar10;
    case 100:
    case 0x69:
      uVar4 = *local_28;
      if ((int)uVar4 < 0) {
        local_38._0_2_ = CONCAT11(0x2d,(byte)local_38);
        local_38 = (uint)(ushort)local_38;
        local_28 = local_28 + 1;
        iVar10 = SPRINTF_OBJ_390(-uVar4);
        return iVar10;
      }
      if ((local_38 >> 1 & 1) != 0) {
        local_38._0_2_ = CONCAT11(0x2b,(byte)local_38);
        local_38 = (uint)(ushort)local_38;
        local_28 = local_28 + 1;
        iVar10 = SPRINTF_OBJ_390();
        return iVar10;
      }
      goto code_r0x80053394;
    case 0x68:
      iVar10 = SPRINTF_OBJ_2E4();
      return iVar10;
    case 0x6c:
      iVar10 = SPRINTF_OBJ_2E4();
      return iVar10;
    case 0x6e:
      piVar9 = (int *)*local_28;
      local_28 = local_28 + 1;
      *piVar9 = iVar10;
      iVar10 = SPRINTF_OBJ_82C();
      return iVar10;
    case 0x6f:
      uVar4 = *local_28;
      local_28 = local_28 + 1;
      if ((local_38 >> 4 & 1) == 0) {
        if ((local_38 >> 3 & 1) != 0) {
          local_30 = local_34;
        }
        if ((int)local_30 < 1) {
          local_30 = 1;
        }
      }
      uVar7 = 0;
      for (; uVar4 != 0; uVar4 = uVar4 >> 3) {
        pbVar8 = pbVar8 + -1;
        *pbVar8 = ((byte)uVar4 & 7) + 0x30;
        uVar7 = uVar7 + 1;
      }
      if ((((local_38 >> 2 & 1) != 0) && (uVar7 != 0)) && (*pbVar8 != 0x30)) {
        pbVar8 = pbVar8 + -1;
        *pbVar8 = 0x30;
        uVar7 = uVar7 + 1;
      }
      if ((int)uVar7 < (int)local_30) {
        do {
          pbVar8 = pbVar8 + -1;
          *pbVar8 = 0x30;
          uVar7 = uVar7 + 1;
        } while ((int)uVar7 < (int)local_30);
        iVar10 = SPRINTF_OBJ_7A0();
        return iVar10;
      }
      break;
    case 0x70:
      local_30 = 8;
      local_38 = local_38 | 0x50;
    case 0x58:
      iVar10 = SPRINTF_OBJ_5A4();
      return iVar10;
    case 0x73:
      pbVar8 = (byte *)*local_28;
      local_28 = local_28 + 1;
      if ((local_38 >> 2 & 1) == 0) {
        if ((local_38 >> 4 & 1) == 0) {
          FUN_80052544(pbVar8);
          iVar10 = SPRINTF_OBJ_7A0();
          return iVar10;
        }
        pvVar3 = memchr(pbVar8,'\0',local_30);
        uVar7 = (int)pvVar3 - (int)pbVar8;
        if (pvVar3 == (void *)0x0) {
          iVar10 = SPRINTF_OBJ_7A0();
          return iVar10;
        }
      }
      else {
        uVar7 = (uint)*pbVar8;
        pbVar8 = pbVar8 + 1;
        if (((local_38 >> 4 & 1) != 0) && ((int)local_30 < (int)uVar7)) {
          iVar10 = SPRINTF_OBJ_7A0();
          return iVar10;
        }
      }
      break;
    case 0x75:
      uVar4 = *local_28;
      local_38 = local_38 & 0xff;
code_r0x80053394:
      local_28 = local_28 + 1;
      if ((local_38 >> 4 & 1) == 0) {
        if (((local_38 >> 3 & 1) != 0) && (local_30 = local_34, local_38._1_1_ != 0)) {
          local_30 = local_34 + -1;
        }
        if ((int)local_30 < 1) {
          local_30 = 1;
        }
      }
      uVar7 = 0;
      while (uVar4 != 0) {
        pbVar8 = pbVar8 + -1;
        uVar7 = uVar7 + 1;
        *pbVar8 = (char)uVar4 + (char)(uVar4 / 10) * -10 + 0x30;
        uVar4 = uVar4 / 10;
      }
      for (; (int)uVar7 < (int)local_30; uVar7 = uVar7 + 1) {
        pbVar8 = pbVar8 + -1;
        *pbVar8 = 0x30;
      }
      if (local_38._1_1_ != 0) {
        pbVar8[-1] = local_38._1_1_;
        iVar10 = SPRINTF_OBJ_7A0();
        return iVar10;
      }
      break;
    case 0x78:
      uVar4 = *local_28;
      local_28 = local_28 + 1;
      if ((local_38 >> 4 & 1) == 0) {
        if (((local_38 >> 3 & 1) != 0) && (local_30 = local_34, (local_38 >> 2 & 1) != 0)) {
          local_30 = local_34 + -2;
        }
        if ((int)local_30 < 1) {
          local_30 = 1;
        }
      }
      uVar7 = 0;
      for (; uVar4 != 0; uVar4 = uVar4 >> 4) {
        pbVar8 = pbVar8 + -1;
        uVar7 = uVar7 + 1;
        *pbVar8 = "0123456789abcdef"[uVar4 & 0xf];
      }
      for (; (int)uVar7 < (int)local_30; uVar7 = uVar7 + 1) {
        pbVar8 = pbVar8 + -1;
        *pbVar8 = 0x30;
      }
      if ((local_38 >> 2 & 1) != 0) {
        pbVar8[-1] = (byte)iVar6;
        pbVar8[-2] = 0x30;
        iVar10 = SPRINTF_OBJ_7A0();
        return iVar10;
      }
    }
    pcVar2 = buffer + iVar10;
    if (((int)uVar7 < (int)local_34) && ((local_38 & 1) == 0)) {
      do {
        buffer[iVar10] = ' ';
        local_34 = local_34 + -1;
        iVar10 = iVar10 + 1;
      } while ((int)uVar7 < (int)local_34);
      pcVar2 = buffer + iVar10;
    }
    FUN_80044d9c(pcVar2,pbVar8,uVar7);
    iVar10 = iVar10 + uVar7;
    for (; (int)uVar7 < (int)local_34; uVar7 = uVar7 + 1) {
      buffer[iVar10] = ' ';
      iVar10 = iVar10 + 1;
    }
    fmt = local_res4 + 1;
    cVar1 = local_res4[1];
  }
SPRINTF_OBJ_84C:
  buffer[iVar10] = '\0';
  return iVar10;
}

