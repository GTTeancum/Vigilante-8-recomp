// addr: 0x80053830  name: SPRINTF_OBJ_82C

/* WARNING: Removing unreachable block (ram,0x80053334) */
/* WARNING: Removing unreachable block (ram,0x8005338c) */
/* WARNING: Removing unreachable block (ram,0x800535d4) */
/* WARNING: Removing unreachable block (ram,0x800534ac) */
/* WARNING: Removing unreachable block (ram,0x8005377c) */

int SPRINTF_OBJ_82C(void)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  bool bVar8;
  byte bVar9;
  int iVar10;
  void *pvVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  byte *pbVar16;
  int unaff_s2;
  int unaff_s3;
  int unaff_s4;
  int unaff_s5;
  int unaff_s6;
  byte bVar17;
  int iVar18;
  int iVar19;
  uint *in_stack_00000220;
  int in_stack_0000024c;
  
  while( true ) {
    iVar14 = (int)*(char *)(in_stack_0000024c + 1);
    if (iVar14 == 0) break;
    if (iVar14 != 0x25) {
SPRINTF_OBJ_794:
      *(char *)(unaff_s3 + unaff_s2) = (char)iVar14;
      iVar14 = SPRINTF_OBJ_82C();
      return iVar14;
    }
    bVar1 = false;
    iVar18 = 0;
    iVar19 = 0;
    iVar10 = in_stack_0000024c + 1;
    bVar6 = false;
    bVar7 = false;
    bVar8 = false;
    bVar9 = 0;
    while( true ) {
      while( true ) {
        do {
          while( true ) {
            while( true ) {
              bVar17 = bVar9;
              bVar5 = bVar8;
              bVar4 = bVar7;
              bVar2 = bVar6;
              iVar12 = iVar10;
              bVar3 = false;
              iVar10 = iVar12 + 1;
              iVar14 = (int)(char)*(byte *)(iVar12 + 1);
              bVar7 = bVar4;
              bVar8 = bVar5;
              bVar9 = bVar17;
              if (iVar14 != unaff_s5) break;
              bVar1 = true;
              bVar6 = bVar2;
            }
            if (iVar14 != unaff_s6) break;
            bVar6 = true;
          }
          bVar6 = false;
          bVar7 = false;
          bVar8 = false;
          bVar9 = *(byte *)(iVar12 + 1);
        } while (iVar14 == unaff_s4);
        if (iVar14 != 0x23) break;
        bVar6 = bVar2;
        bVar7 = bVar4;
        bVar8 = true;
        bVar9 = bVar17;
      }
      if (iVar14 != 0x30) break;
      bVar6 = bVar2;
      bVar7 = true;
      bVar8 = bVar5;
      bVar9 = bVar17;
    }
    if (iVar14 == 0x2a) {
      iVar14 = SPRINTF_OBJ_1BC(*in_stack_00000220,(int)*(char *)(iVar12 + 2));
      return iVar14;
    }
    while (iVar14 - 0x30U < 10) {
      iVar18 = iVar18 * 10 + -0x30 + iVar14;
      iVar14 = (int)*(char *)(iVar10 + 1);
      iVar10 = iVar10 + 1;
    }
    iVar12 = 0x23;
    in_stack_0000024c = iVar10;
    if (iVar14 == 0x2e) {
      iVar14 = (int)*(char *)(iVar10 + 1);
      iVar12 = iVar10;
      if (iVar14 == 0x2a) {
        iVar14 = SPRINTF_OBJ_258(iVar10,(int)*(char *)(iVar10 + 2));
        return iVar14;
      }
      while (in_stack_0000024c = iVar12 + 1, iVar14 - 0x30U < 10) {
        iVar19 = iVar19 * 10 + -0x30 + iVar14;
        iVar14 = (int)*(char *)(iVar12 + 2);
        iVar12 = in_stack_0000024c;
      }
      iVar12 = iVar10;
      if (-1 < iVar19) {
        bVar3 = true;
      }
    }
    pbVar16 = &stack0x00000210;
    if (bVar1) {
      bVar4 = false;
    }
    switch(iVar14) {
    case 0x4c:
      iVar14 = SPRINTF_OBJ_29C(iVar12,(int)*(char *)(in_stack_0000024c + 1));
      return iVar14;
    default:
      if (iVar14 == 0x25) goto SPRINTF_OBJ_794;
      goto SPRINTF_OBJ_84C;
    case 99:
      iVar14 = SPRINTF_OBJ_7A0();
      return iVar14;
    case 100:
    case 0x69:
      uVar13 = *in_stack_00000220;
      if ((int)uVar13 < 0) {
        iVar14 = SPRINTF_OBJ_390(-uVar13);
        return iVar14;
      }
      if (bVar2) {
        iVar14 = SPRINTF_OBJ_390();
        return iVar14;
      }
      goto code_r0x80053394;
    case 0x68:
      iVar14 = SPRINTF_OBJ_2E4();
      return iVar14;
    case 0x6c:
      iVar14 = SPRINTF_OBJ_2E4();
      return iVar14;
    case 0x6e:
      *(int *)*in_stack_00000220 = unaff_s2;
      iVar14 = SPRINTF_OBJ_82C();
      return iVar14;
    case 0x6f:
      uVar13 = *in_stack_00000220;
      if (!bVar3) {
        if (bVar4) {
          iVar19 = iVar18;
        }
        if (iVar19 < 1) {
          iVar19 = 1;
        }
      }
      uVar15 = 0;
      for (; uVar13 != 0; uVar13 = uVar13 >> 3) {
        pbVar16 = pbVar16 + -1;
        *pbVar16 = ((byte)uVar13 & 7) + 0x30;
        uVar15 = uVar15 + 1;
      }
      if (((bVar5) && (uVar15 != 0)) && (*pbVar16 != 0x30)) {
        pbVar16 = pbVar16 + -1;
        *pbVar16 = 0x30;
        uVar15 = uVar15 + 1;
      }
      if ((int)uVar15 < iVar19) {
        do {
          pbVar16 = pbVar16 + -1;
          *pbVar16 = 0x30;
          uVar15 = uVar15 + 1;
        } while ((int)uVar15 < iVar19);
        iVar14 = SPRINTF_OBJ_7A0();
        return iVar14;
      }
      break;
    case 0x70:
    case 0x58:
      iVar14 = SPRINTF_OBJ_5A4();
      return iVar14;
    case 0x73:
      pbVar16 = (byte *)*in_stack_00000220;
      if (bVar5) {
        uVar15 = (uint)*pbVar16;
        pbVar16 = pbVar16 + 1;
        if ((bVar3) && (iVar19 < (int)uVar15)) {
          iVar14 = SPRINTF_OBJ_7A0();
          return iVar14;
        }
      }
      else {
        if (!bVar3) {
          FUN_80052544(pbVar16);
          iVar14 = SPRINTF_OBJ_7A0();
          return iVar14;
        }
        pvVar11 = memchr(pbVar16,'\0',iVar19);
        uVar15 = (int)pvVar11 - (int)pbVar16;
        if (pvVar11 == (void *)0x0) {
          iVar14 = SPRINTF_OBJ_7A0();
          return iVar14;
        }
      }
      break;
    case 0x75:
      uVar13 = *in_stack_00000220;
      bVar17 = 0;
code_r0x80053394:
      if (!bVar3) {
        if ((bVar4) && (iVar19 = iVar18, bVar17 != 0)) {
          iVar19 = iVar18 + -1;
        }
        if (iVar19 < 1) {
          iVar19 = 1;
        }
      }
      uVar15 = 0;
      while (uVar13 != 0) {
        pbVar16 = pbVar16 + -1;
        uVar15 = uVar15 + 1;
        *pbVar16 = (char)uVar13 + (char)(uVar13 / 10) * -10 + 0x30;
        uVar13 = uVar13 / 10;
      }
      for (; (int)uVar15 < iVar19; uVar15 = uVar15 + 1) {
        pbVar16 = pbVar16 + -1;
        *pbVar16 = 0x30;
      }
      if (bVar17 != 0) {
        pbVar16[-1] = bVar17;
        iVar14 = SPRINTF_OBJ_7A0();
        return iVar14;
      }
      break;
    case 0x78:
      uVar13 = *in_stack_00000220;
      if (!bVar3) {
        if ((bVar4) && (iVar19 = iVar18, bVar5)) {
          iVar19 = iVar18 + -2;
        }
        if (iVar19 < 1) {
          iVar19 = 1;
        }
      }
      uVar15 = 0;
      for (; uVar13 != 0; uVar13 = uVar13 >> 4) {
        pbVar16 = pbVar16 + -1;
        uVar15 = uVar15 + 1;
        *pbVar16 = "0123456789abcdef"[uVar13 & 0xf];
      }
      for (; (int)uVar15 < iVar19; uVar15 = uVar15 + 1) {
        pbVar16 = pbVar16 + -1;
        *pbVar16 = 0x30;
      }
      if (bVar5) {
        pbVar16[-1] = (byte)iVar14;
        pbVar16[-2] = 0x30;
        iVar14 = SPRINTF_OBJ_7A0();
        return iVar14;
      }
    }
    in_stack_00000220 = in_stack_00000220 + 1;
    iVar14 = unaff_s3 + unaff_s2;
    if (((int)uVar15 < iVar18) && (!bVar1)) {
      do {
        *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
        iVar18 = iVar18 + -1;
        unaff_s2 = unaff_s2 + 1;
      } while ((int)uVar15 < iVar18);
      iVar14 = unaff_s3 + unaff_s2;
    }
    FUN_80044d9c(iVar14,pbVar16,uVar15);
    unaff_s2 = unaff_s2 + uVar15;
    for (; (int)uVar15 < iVar18; uVar15 = uVar15 + 1) {
      *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
      unaff_s2 = unaff_s2 + 1;
    }
  }
SPRINTF_OBJ_84C:
  *(undefined1 *)(unaff_s3 + unaff_s2) = 0;
  return unaff_s2;
}

