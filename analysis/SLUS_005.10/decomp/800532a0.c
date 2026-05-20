// addr: 0x800532a0  name: SPRINTF_OBJ_29C

int SPRINTF_OBJ_29C(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int in_v1;
  uint uVar4;
  uint uVar5;
  byte *unaff_s1;
  int unaff_s2;
  int unaff_s3;
  int unaff_s4;
  int unaff_s5;
  int unaff_s6;
  byte bVar6;
  uint in_stack_00000210;
  int in_stack_00000214;
  int in_stack_00000218;
  uint *in_stack_00000220;
  int in_stack_0000024c;
  
  do {
    switch(in_v1) {
    case 0:
      iVar3 = SPRINTF_OBJ_29C(param_1,(int)*(char *)(in_stack_0000024c + 1));
      return iVar3;
    default:
      if (param_2 == 0x25) goto SPRINTF_OBJ_794;
      goto SPRINTF_OBJ_84C;
    case 0x17:
      unaff_s1[-1] = (byte)*in_stack_00000220;
      iVar3 = SPRINTF_OBJ_7A0();
      return iVar3;
    case 0x18:
    case 0x1d:
      uVar4 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar4 = (uint)(short)uVar4;
      }
      if ((int)uVar4 < 0) {
        iVar3 = SPRINTF_OBJ_390(-uVar4);
        return iVar3;
      }
      if ((in_stack_00000210 >> 1 & 1) != 0) {
        iVar3 = SPRINTF_OBJ_390();
        return iVar3;
      }
      goto code_r0x80053394;
    case 0x1c:
      iVar3 = SPRINTF_OBJ_2E4();
      return iVar3;
    case 0x20:
      iVar3 = SPRINTF_OBJ_2E4();
      return iVar3;
    case 0x22:
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        *(short *)*in_stack_00000220 = (short)unaff_s2;
        iVar3 = SPRINTF_OBJ_82C();
        return iVar3;
      }
      *(int *)*in_stack_00000220 = unaff_s2;
      iVar3 = SPRINTF_OBJ_82C();
      return iVar3;
    case 0x23:
      uVar4 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar4 = uVar4 & 0xffff;
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
      for (; uVar4 != 0; uVar4 = uVar4 >> 3) {
        unaff_s1 = unaff_s1 + -1;
        *unaff_s1 = ((byte)uVar4 & 7) + 0x30;
        uVar5 = uVar5 + 1;
      }
      if ((((in_stack_00000210 >> 2 & 1) != 0) && (uVar5 != 0)) && (*unaff_s1 != 0x30)) {
        unaff_s1 = unaff_s1 + -1;
        *unaff_s1 = 0x30;
        uVar5 = uVar5 + 1;
      }
      if ((int)uVar5 < in_stack_00000218) {
        do {
          unaff_s1 = unaff_s1 + -1;
          *unaff_s1 = 0x30;
          uVar5 = uVar5 + 1;
        } while ((int)uVar5 < in_stack_00000218);
        iVar3 = SPRINTF_OBJ_7A0();
        return iVar3;
      }
      break;
    case 0x24:
    case 0xc:
      iVar3 = SPRINTF_OBJ_5A4();
      return iVar3;
    case 0x27:
      unaff_s1 = (byte *)*in_stack_00000220;
      if ((in_stack_00000210 >> 2 & 1) == 0) {
        if ((in_stack_00000210 >> 4 & 1) == 0) {
          FUN_80052544(unaff_s1);
          iVar3 = SPRINTF_OBJ_7A0();
          return iVar3;
        }
        pvVar2 = memchr(unaff_s1,'\0',in_stack_00000218);
        uVar5 = (int)pvVar2 - (int)unaff_s1;
        if (pvVar2 == (void *)0x0) {
          iVar3 = SPRINTF_OBJ_7A0();
          return iVar3;
        }
      }
      else {
        uVar5 = (uint)*unaff_s1;
        unaff_s1 = unaff_s1 + 1;
        if (((in_stack_00000210 >> 4 & 1) != 0) && (in_stack_00000218 < (int)uVar5)) {
          iVar3 = SPRINTF_OBJ_7A0();
          return iVar3;
        }
      }
      break;
    case 0x29:
      uVar4 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar4 = uVar4 & 0xffff;
      }
      in_stack_00000210 = in_stack_00000210 & 0xff;
code_r0x80053394:
      bVar6 = (byte)(in_stack_00000210 >> 8);
      if ((in_stack_00000210 >> 4 & 1) == 0) {
        if (((in_stack_00000210 >> 3 & 1) != 0) &&
           (in_stack_00000218 = in_stack_00000214, bVar6 != 0)) {
          in_stack_00000218 = in_stack_00000214 + -1;
        }
        if (in_stack_00000218 < 1) {
          in_stack_00000218 = 1;
        }
      }
      uVar5 = 0;
      while (uVar4 != 0) {
        unaff_s1 = unaff_s1 + -1;
        uVar5 = uVar5 + 1;
        *unaff_s1 = (char)uVar4 + (char)(uVar4 / 10) * -10 + 0x30;
        uVar4 = uVar4 / 10;
      }
      for (; (int)uVar5 < in_stack_00000218; uVar5 = uVar5 + 1) {
        unaff_s1 = unaff_s1 + -1;
        *unaff_s1 = 0x30;
      }
      if (bVar6 != 0) {
        unaff_s1[-1] = bVar6;
        iVar3 = SPRINTF_OBJ_7A0();
        return iVar3;
      }
      break;
    case 0x2c:
      uVar4 = *in_stack_00000220;
      if ((in_stack_00000210 >> 5 & 1) != 0) {
        uVar4 = uVar4 & 0xffff;
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
      for (; uVar4 != 0; uVar4 = uVar4 >> 4) {
        unaff_s1 = unaff_s1 + -1;
        uVar5 = uVar5 + 1;
        *unaff_s1 = "0123456789abcdef"[uVar4 & 0xf];
      }
      for (; (int)uVar5 < in_stack_00000218; uVar5 = uVar5 + 1) {
        unaff_s1 = unaff_s1 + -1;
        *unaff_s1 = 0x30;
      }
      if ((in_stack_00000210 >> 2 & 1) != 0) {
        unaff_s1[-1] = (byte)param_2;
        unaff_s1[-2] = 0x30;
        iVar3 = SPRINTF_OBJ_7A0();
        return iVar3;
      }
    }
    in_stack_00000220 = in_stack_00000220 + 1;
    iVar3 = unaff_s3 + unaff_s2;
    if (((int)uVar5 < in_stack_00000214) && ((in_stack_00000210 & 1) == 0)) {
      do {
        *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
        in_stack_00000214 = in_stack_00000214 + -1;
        unaff_s2 = unaff_s2 + 1;
      } while ((int)uVar5 < in_stack_00000214);
      iVar3 = unaff_s3 + unaff_s2;
    }
    FUN_80044d9c(iVar3,unaff_s1,uVar5);
    unaff_s2 = unaff_s2 + uVar5;
    for (; (int)uVar5 < in_stack_00000214; uVar5 = uVar5 + 1) {
      *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
      unaff_s2 = unaff_s2 + 1;
    }
    param_2 = (int)*(char *)(in_stack_0000024c + 1);
    if (param_2 == 0) {
SPRINTF_OBJ_84C:
      *(undefined1 *)(unaff_s3 + unaff_s2) = 0;
      return unaff_s2;
    }
    if (param_2 != 0x25) {
SPRINTF_OBJ_794:
      *(char *)(unaff_s3 + unaff_s2) = (char)param_2;
      iVar3 = SPRINTF_OBJ_82C();
      return iVar3;
    }
    in_stack_00000210 = 0;
    in_stack_00000214 = 0;
    in_stack_00000218 = 0;
    iVar3 = in_stack_0000024c + 1;
    while( true ) {
      while( true ) {
        while( true ) {
          while( true ) {
            while( true ) {
              iVar1 = iVar3 + 1;
              param_2 = (int)*(char *)(iVar3 + 1);
              if (param_2 != unaff_s5) break;
              in_stack_00000210 = in_stack_00000210 | 1;
              iVar3 = iVar1;
            }
            if (param_2 != unaff_s6) break;
            in_stack_00000210 = in_stack_00000210 | 2;
            iVar3 = iVar1;
          }
          if (param_2 != unaff_s4) break;
          in_stack_00000210 = (uint)CONCAT11(*(char *)(iVar3 + 1),(byte)in_stack_00000210);
          iVar3 = iVar1;
        }
        if (param_2 != 0x23) break;
        in_stack_00000210 = in_stack_00000210 | 4;
        iVar3 = iVar1;
      }
      if (param_2 != 0x30) break;
      in_stack_00000210 = in_stack_00000210 | 8;
      iVar3 = iVar1;
    }
    if (param_2 == 0x2a) {
      iVar3 = SPRINTF_OBJ_1BC(*in_stack_00000220,(int)*(char *)(iVar3 + 2));
      return iVar3;
    }
    while (param_2 - 0x30U < 10) {
      in_stack_00000214 = in_stack_00000214 * 10 + -0x30 + param_2;
      param_2 = (int)*(char *)(iVar1 + 1);
      iVar1 = iVar1 + 1;
    }
    param_1 = 0x23;
    in_stack_0000024c = iVar1;
    if (param_2 == 0x2e) {
      param_2 = (int)*(char *)(iVar1 + 1);
      iVar3 = iVar1;
      if (param_2 == 0x2a) {
        iVar3 = SPRINTF_OBJ_258(iVar1,(int)*(char *)(iVar1 + 2));
        return iVar3;
      }
      while (in_stack_0000024c = iVar3 + 1, param_2 - 0x30U < 10) {
        in_stack_00000218 = in_stack_00000218 * 10 + -0x30 + param_2;
        param_2 = (int)*(char *)(iVar3 + 2);
        iVar3 = in_stack_0000024c;
      }
      param_1 = iVar1;
      if (-1 < in_stack_00000218) {
        in_stack_00000210 = in_stack_00000210 | 0x10;
      }
    }
    unaff_s1 = &stack0x00000210;
    if ((in_stack_00000210 & 1) != 0) {
      in_stack_00000210 = in_stack_00000210 & 0xfffffff7;
    }
    in_v1 = param_2 + -0x4c;
  } while( true );
}

