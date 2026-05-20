// addr: 0x80053094  name: SPRINTF_OBJ_90

int SPRINTF_OBJ_90(int param_1)

{
  int iVar1;
  void *pvVar2;
  int in_v1;
  uint uVar3;
  int iVar4;
  uint uVar5;
  byte *pbVar6;
  int unaff_s2;
  int unaff_s3;
  int unaff_s4;
  int unaff_s5;
  int unaff_s6;
  byte bVar7;
  uint in_stack_00000210;
  int in_stack_00000214;
  int in_stack_00000218;
  uint *in_stack_00000220;
  int in_stack_0000024c;
  int iVar8;
  int iVar9;
  
  do {
    while( true ) {
      while( true ) {
        while( true ) {
          while( true ) {
            while( true ) {
              iVar1 = in_stack_0000024c + 1;
              iVar4 = (int)*(char *)(in_stack_0000024c + 1);
              if (iVar4 != unaff_s5) break;
              in_stack_00000210 = in_stack_00000210 | 1;
              in_stack_0000024c = iVar1;
            }
            if (iVar4 != unaff_s6) break;
            in_stack_00000210 = in_stack_00000210 | 2;
            in_stack_0000024c = iVar1;
          }
          if (iVar4 != unaff_s4) break;
          in_stack_00000210 =
               (uint)CONCAT11(*(char *)(in_stack_0000024c + 1),(byte)in_stack_00000210);
          in_stack_0000024c = iVar1;
        }
        if (iVar4 != param_1) break;
        in_stack_00000210 = in_stack_00000210 | 4;
        in_stack_0000024c = iVar1;
      }
      if (iVar4 != in_v1) break;
      in_stack_00000210 = in_stack_00000210 | 8;
      in_stack_0000024c = iVar1;
    }
    if (iVar4 == 0x2a) {
      iVar1 = SPRINTF_OBJ_1BC(*in_stack_00000220,(int)*(char *)(in_stack_0000024c + 2));
      return iVar1;
    }
    while (iVar4 - 0x30U < 10) {
      in_stack_00000214 = in_stack_00000214 * 10 + -0x30 + iVar4;
      iVar4 = (int)*(char *)(iVar1 + 1);
      iVar1 = iVar1 + 1;
    }
    iVar8 = iVar1;
    if (iVar4 == 0x2e) {
      iVar4 = (int)*(char *)(iVar1 + 1);
      iVar9 = iVar1;
      if (iVar4 == 0x2a) {
        iVar1 = SPRINTF_OBJ_258(iVar1,(int)*(char *)(iVar1 + 2));
        return iVar1;
      }
      while (iVar8 = iVar9 + 1, iVar4 - 0x30U < 10) {
        in_stack_00000218 = in_stack_00000218 * 10 + -0x30 + iVar4;
        iVar4 = (int)*(char *)(iVar9 + 2);
        iVar9 = iVar8;
      }
      param_1 = iVar1;
      if (-1 < in_stack_00000218) {
        in_stack_00000210 = in_stack_00000210 | 0x10;
      }
    }
    pbVar6 = &stack0x00000210;
    if ((in_stack_00000210 & 1) != 0) {
      in_stack_00000210 = in_stack_00000210 & 0xfffffff7;
    }
    switch(iVar4) {
    case 0x4c:
      iVar1 = SPRINTF_OBJ_29C(param_1,(int)*(char *)(iVar8 + 1));
      return iVar1;
    default:
      if (iVar4 != 0x25) goto SPRINTF_OBJ_84C;
SPRINTF_OBJ_794:
      *(char *)(unaff_s3 + unaff_s2) = (char)iVar4;
      iVar1 = SPRINTF_OBJ_82C();
      return iVar1;
    case 99:
      iVar1 = SPRINTF_OBJ_7A0();
      return iVar1;
    case 100:
    case 0x69:
      uVar3 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar3 = (uint)(short)uVar3;
      }
      if ((int)uVar3 < 0) {
        iVar1 = SPRINTF_OBJ_390(-uVar3);
        return iVar1;
      }
      if ((in_stack_00000210 >> 1 & 1) != 0) {
        iVar1 = SPRINTF_OBJ_390();
        return iVar1;
      }
      goto code_r0x80053394;
    case 0x68:
      iVar1 = SPRINTF_OBJ_2E4();
      return iVar1;
    case 0x6c:
      iVar1 = SPRINTF_OBJ_2E4();
      return iVar1;
    case 0x6e:
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        *(short *)*in_stack_00000220 = (short)unaff_s2;
        iVar1 = SPRINTF_OBJ_82C();
        return iVar1;
      }
      *(int *)*in_stack_00000220 = unaff_s2;
      iVar1 = SPRINTF_OBJ_82C();
      return iVar1;
    case 0x6f:
      uVar3 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar3 = uVar3 & 0xffff;
      }
      if ((in_stack_00000210 >> 4 & 1) == 0) {
        if ((in_stack_00000210 >> 3 & 1) != 0) {
          in_stack_00000218 = in_stack_00000214;
        }
        if (in_stack_00000218 < 1) {
          in_stack_00000218 = 1;
        }
      }
      uVar5 = 0;
      for (; uVar3 != 0; uVar3 = uVar3 >> 3) {
        pbVar6 = pbVar6 + -1;
        *pbVar6 = ((byte)uVar3 & 7) + 0x30;
        uVar5 = uVar5 + 1;
      }
      if ((((in_stack_00000210 >> 2 & 1) != 0) && (uVar5 != 0)) && (*pbVar6 != 0x30)) {
        pbVar6 = pbVar6 + -1;
        *pbVar6 = 0x30;
        uVar5 = uVar5 + 1;
      }
      if ((int)uVar5 < in_stack_00000218) {
        do {
          pbVar6 = pbVar6 + -1;
          *pbVar6 = 0x30;
          uVar5 = uVar5 + 1;
        } while ((int)uVar5 < in_stack_00000218);
        iVar1 = SPRINTF_OBJ_7A0();
        return iVar1;
      }
      break;
    case 0x70:
    case 0x58:
      iVar1 = SPRINTF_OBJ_5A4();
      return iVar1;
    case 0x73:
      pbVar6 = (byte *)*in_stack_00000220;
      if ((in_stack_00000210 >> 2 & 1) == 0) {
        if ((in_stack_00000210 >> 4 & 1) == 0) {
          FUN_80052544(pbVar6);
          iVar1 = SPRINTF_OBJ_7A0();
          return iVar1;
        }
        pvVar2 = memchr(pbVar6,'\0',in_stack_00000218);
        uVar5 = (int)pvVar2 - (int)pbVar6;
        if (pvVar2 == (void *)0x0) {
          iVar1 = SPRINTF_OBJ_7A0();
          return iVar1;
        }
      }
      else {
        uVar5 = (uint)*pbVar6;
        pbVar6 = pbVar6 + 1;
        if (((in_stack_00000210 >> 4 & 1) != 0) && (in_stack_00000218 < (int)uVar5)) {
          iVar1 = SPRINTF_OBJ_7A0();
          return iVar1;
        }
      }
      break;
    case 0x75:
      uVar3 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar3 = uVar3 & 0xffff;
      }
      in_stack_00000210 = in_stack_00000210 & 0xff;
code_r0x80053394:
      bVar7 = (byte)(in_stack_00000210 >> 8);
      if ((in_stack_00000210 >> 4 & 1) == 0) {
        if (((in_stack_00000210 >> 3 & 1) != 0) &&
           (in_stack_00000218 = in_stack_00000214, bVar7 != 0)) {
          in_stack_00000218 = in_stack_00000214 + -1;
        }
        if (in_stack_00000218 < 1) {
          in_stack_00000218 = 1;
        }
      }
      uVar5 = 0;
      while (uVar3 != 0) {
        pbVar6 = pbVar6 + -1;
        uVar5 = uVar5 + 1;
        *pbVar6 = (char)uVar3 + (char)(uVar3 / 10) * -10 + 0x30;
        uVar3 = uVar3 / 10;
      }
      for (; (int)uVar5 < in_stack_00000218; uVar5 = uVar5 + 1) {
        pbVar6 = pbVar6 + -1;
        *pbVar6 = 0x30;
      }
      if (bVar7 != 0) {
        pbVar6[-1] = bVar7;
        iVar1 = SPRINTF_OBJ_7A0();
        return iVar1;
      }
      break;
    case 0x78:
      uVar3 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar3 = uVar3 & 0xffff;
      }
      if ((in_stack_00000210 >> 4 & 1) == 0) {
        if (((in_stack_00000210 >> 3 & 1) != 0) &&
           (in_stack_00000218 = in_stack_00000214, (in_stack_00000210 >> 2 & 1) != 0)) {
          in_stack_00000218 = in_stack_00000214 + -2;
        }
        if (in_stack_00000218 < 1) {
          in_stack_00000218 = 1;
        }
      }
      uVar5 = 0;
      for (; uVar3 != 0; uVar3 = uVar3 >> 4) {
        pbVar6 = pbVar6 + -1;
        uVar5 = uVar5 + 1;
        *pbVar6 = "0123456789abcdef"[uVar3 & 0xf];
      }
      for (; (int)uVar5 < in_stack_00000218; uVar5 = uVar5 + 1) {
        pbVar6 = pbVar6 + -1;
        *pbVar6 = 0x30;
      }
      if ((in_stack_00000210 >> 2 & 1) != 0) {
        pbVar6[-1] = (byte)iVar4;
        pbVar6[-2] = 0x30;
        iVar1 = SPRINTF_OBJ_7A0();
        return iVar1;
      }
    }
    in_stack_00000220 = in_stack_00000220 + 1;
    iVar1 = unaff_s3 + unaff_s2;
    if (((int)uVar5 < in_stack_00000214) && ((in_stack_00000210 & 1) == 0)) {
      do {
        *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
        in_stack_00000214 = in_stack_00000214 + -1;
        unaff_s2 = unaff_s2 + 1;
      } while ((int)uVar5 < in_stack_00000214);
      iVar1 = unaff_s3 + unaff_s2;
    }
    FUN_80044d9c(iVar1,pbVar6,uVar5);
    unaff_s2 = unaff_s2 + uVar5;
    for (; (int)uVar5 < in_stack_00000214; uVar5 = uVar5 + 1) {
      *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
      unaff_s2 = unaff_s2 + 1;
    }
    in_stack_0000024c = iVar8 + 1;
    iVar4 = (int)*(char *)(iVar8 + 1);
    if (iVar4 == 0) {
SPRINTF_OBJ_84C:
      *(undefined1 *)(unaff_s3 + unaff_s2) = 0;
      return unaff_s2;
    }
    if (iVar4 != 0x25) goto SPRINTF_OBJ_794;
    in_stack_00000210 = 0;
    in_stack_00000214 = 0;
    in_stack_00000218 = 0;
    param_1 = 0x23;
    in_v1 = 0x30;
  } while( true );
}

