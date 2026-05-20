// addr: 0x800537a4  name: SPRINTF_OBJ_7A0

/* WARNING: Removing unreachable block (ram,0x80053334) */
/* WARNING: Removing unreachable block (ram,0x8005338c) */
/* WARNING: Removing unreachable block (ram,0x800535d4) */
/* WARNING: Removing unreachable block (ram,0x800534ac) */
/* WARNING: Removing unreachable block (ram,0x8005377c) */

int SPRINTF_OBJ_7A0(void)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint unaff_s0;
  byte *unaff_s1;
  int unaff_s2;
  int unaff_s3;
  int unaff_s4;
  int unaff_s5;
  int unaff_s6;
  byte bVar6;
  uint in_stack_00000210;
  int in_stack_00000214;
  int iVar7;
  uint *in_stack_00000220;
  int in_stack_0000024c;
  
code_r0x800537a4:
  iVar5 = unaff_s3 + unaff_s2;
  if (((int)unaff_s0 < in_stack_00000214) && ((in_stack_00000210 & 1) == 0)) {
    do {
      *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
      in_stack_00000214 = in_stack_00000214 + -1;
      unaff_s2 = unaff_s2 + 1;
    } while ((int)unaff_s0 < in_stack_00000214);
    iVar5 = unaff_s3 + unaff_s2;
  }
  FUN_80044d9c(iVar5,unaff_s1,unaff_s0);
  unaff_s2 = unaff_s2 + unaff_s0;
  for (; (int)unaff_s0 < in_stack_00000214; unaff_s0 = unaff_s0 + 1) {
    *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
    unaff_s2 = unaff_s2 + 1;
  }
  iVar5 = (int)*(char *)(in_stack_0000024c + 1);
  if (iVar5 == 0) {
SPRINTF_OBJ_84C:
    *(undefined1 *)(unaff_s3 + unaff_s2) = 0;
    return unaff_s2;
  }
  if (iVar5 != 0x25) {
SPRINTF_OBJ_794:
    *(char *)(unaff_s3 + unaff_s2) = (char)iVar5;
    iVar5 = SPRINTF_OBJ_82C();
    return iVar5;
  }
  in_stack_00000210 = 0;
  in_stack_00000214 = 0;
  iVar7 = 0;
  iVar3 = in_stack_0000024c + 1;
  while( true ) {
    while( true ) {
      while( true ) {
        while( true ) {
          while( true ) {
            iVar1 = iVar3 + 1;
            iVar5 = (int)*(char *)(iVar3 + 1);
            if (iVar5 != unaff_s5) break;
            in_stack_00000210 = in_stack_00000210 | 1;
            iVar3 = iVar1;
          }
          if (iVar5 != unaff_s6) break;
          in_stack_00000210 = in_stack_00000210 | 2;
          iVar3 = iVar1;
        }
        if (iVar5 != unaff_s4) break;
        in_stack_00000210 = (uint)CONCAT11(*(char *)(iVar3 + 1),(byte)in_stack_00000210);
        iVar3 = iVar1;
      }
      if (iVar5 != 0x23) break;
      in_stack_00000210 = in_stack_00000210 | 4;
      iVar3 = iVar1;
    }
    if (iVar5 != 0x30) break;
    in_stack_00000210 = in_stack_00000210 | 8;
    iVar3 = iVar1;
  }
  if (iVar5 == 0x2a) {
    iVar5 = SPRINTF_OBJ_1BC(*in_stack_00000220,(int)*(char *)(iVar3 + 2));
    return iVar5;
  }
  while (iVar5 - 0x30U < 10) {
    in_stack_00000214 = in_stack_00000214 * 10 + -0x30 + iVar5;
    iVar5 = (int)*(char *)(iVar1 + 1);
    iVar1 = iVar1 + 1;
  }
  iVar3 = 0x23;
  in_stack_0000024c = iVar1;
  if (iVar5 == 0x2e) {
    iVar5 = (int)*(char *)(iVar1 + 1);
    iVar3 = iVar1;
    if (iVar5 == 0x2a) {
      iVar5 = SPRINTF_OBJ_258(iVar1,(int)*(char *)(iVar1 + 2));
      return iVar5;
    }
    while (in_stack_0000024c = iVar3 + 1, iVar5 - 0x30U < 10) {
      iVar7 = iVar7 * 10 + -0x30 + iVar5;
      iVar5 = (int)*(char *)(iVar3 + 2);
      iVar3 = in_stack_0000024c;
    }
    iVar3 = iVar1;
    if (-1 < iVar7) {
      in_stack_00000210 = in_stack_00000210 | 0x10;
    }
  }
  unaff_s1 = &stack0x00000210;
  if ((in_stack_00000210 & 1) != 0) {
    in_stack_00000210 = in_stack_00000210 & 0xfffffff7;
  }
  switch(iVar5) {
  case 0x4c:
    iVar5 = SPRINTF_OBJ_29C(iVar3,(int)*(char *)(in_stack_0000024c + 1));
    return iVar5;
  default:
    if (iVar5 != 0x25) goto SPRINTF_OBJ_84C;
    goto SPRINTF_OBJ_794;
  case 99:
    iVar5 = SPRINTF_OBJ_7A0();
    return iVar5;
  case 100:
  case 0x69:
    uVar4 = *in_stack_00000220;
    if ((int)uVar4 < 0) {
      iVar5 = SPRINTF_OBJ_390(-uVar4);
      return iVar5;
    }
    if ((in_stack_00000210 >> 1 & 1) != 0) {
      iVar5 = SPRINTF_OBJ_390();
      return iVar5;
    }
    break;
  case 0x68:
    iVar5 = SPRINTF_OBJ_2E4();
    return iVar5;
  case 0x6c:
    iVar5 = SPRINTF_OBJ_2E4();
    return iVar5;
  case 0x6e:
    *(int *)*in_stack_00000220 = unaff_s2;
    iVar5 = SPRINTF_OBJ_82C();
    return iVar5;
  case 0x6f:
    uVar4 = *in_stack_00000220;
    in_stack_00000220 = in_stack_00000220 + 1;
    if ((in_stack_00000210 >> 4 & 1) == 0) {
      if ((in_stack_00000210 >> 3 & 1) != 0) {
        iVar7 = in_stack_00000214;
      }
      if (iVar7 < 1) {
        iVar7 = 1;
      }
    }
    unaff_s0 = 0;
    for (; uVar4 != 0; uVar4 = uVar4 >> 3) {
      unaff_s1 = unaff_s1 + -1;
      *unaff_s1 = ((byte)uVar4 & 7) + 0x30;
      unaff_s0 = unaff_s0 + 1;
    }
    if ((((in_stack_00000210 >> 2 & 1) != 0) && (unaff_s0 != 0)) && (*unaff_s1 != 0x30)) {
      unaff_s1 = unaff_s1 + -1;
      *unaff_s1 = 0x30;
      unaff_s0 = unaff_s0 + 1;
    }
    if ((int)unaff_s0 < iVar7) {
      do {
        unaff_s1 = unaff_s1 + -1;
        *unaff_s1 = 0x30;
        unaff_s0 = unaff_s0 + 1;
      } while ((int)unaff_s0 < iVar7);
      iVar5 = SPRINTF_OBJ_7A0();
      return iVar5;
    }
    goto code_r0x800537a4;
  case 0x70:
  case 0x58:
    iVar5 = SPRINTF_OBJ_5A4();
    return iVar5;
  case 0x73:
    unaff_s1 = (byte *)*in_stack_00000220;
    in_stack_00000220 = in_stack_00000220 + 1;
    if ((in_stack_00000210 >> 2 & 1) == 0) {
      if ((in_stack_00000210 >> 4 & 1) == 0) {
        FUN_80052544(unaff_s1);
        iVar5 = SPRINTF_OBJ_7A0();
        return iVar5;
      }
      pvVar2 = memchr(unaff_s1,'\0',iVar7);
      unaff_s0 = (int)pvVar2 - (int)unaff_s1;
      if (pvVar2 == (void *)0x0) {
        iVar5 = SPRINTF_OBJ_7A0();
        return iVar5;
      }
    }
    else {
      unaff_s0 = (uint)*unaff_s1;
      unaff_s1 = unaff_s1 + 1;
      if (((in_stack_00000210 >> 4 & 1) != 0) && (iVar7 < (int)unaff_s0)) {
        iVar5 = SPRINTF_OBJ_7A0();
        return iVar5;
      }
    }
    goto code_r0x800537a4;
  case 0x75:
    uVar4 = *in_stack_00000220;
    in_stack_00000210 = in_stack_00000210 & 0xff;
    break;
  case 0x78:
    goto SPRINTF_OBJ_59C;
  }
  in_stack_00000220 = in_stack_00000220 + 1;
  bVar6 = (byte)(in_stack_00000210 >> 8);
  if ((in_stack_00000210 >> 4 & 1) == 0) {
    if (((in_stack_00000210 >> 3 & 1) != 0) && (iVar7 = in_stack_00000214, bVar6 != 0)) {
      iVar7 = in_stack_00000214 + -1;
    }
    if (iVar7 < 1) {
      iVar7 = 1;
    }
  }
  unaff_s0 = 0;
  while (uVar4 != 0) {
    unaff_s1 = unaff_s1 + -1;
    unaff_s0 = unaff_s0 + 1;
    *unaff_s1 = (char)uVar4 + (char)(uVar4 / 10) * -10 + 0x30;
    uVar4 = uVar4 / 10;
  }
  for (; (int)unaff_s0 < iVar7; unaff_s0 = unaff_s0 + 1) {
    unaff_s1 = unaff_s1 + -1;
    *unaff_s1 = 0x30;
  }
  if (bVar6 != 0) {
    unaff_s1[-1] = bVar6;
    iVar5 = SPRINTF_OBJ_7A0();
    return iVar5;
  }
  goto code_r0x800537a4;
SPRINTF_OBJ_59C:
  uVar4 = *in_stack_00000220;
  in_stack_00000220 = in_stack_00000220 + 1;
  if ((in_stack_00000210 >> 4 & 1) == 0) {
    if (((in_stack_00000210 >> 3 & 1) != 0) &&
       (iVar7 = in_stack_00000214, (in_stack_00000210 >> 2 & 1) != 0)) {
      iVar7 = in_stack_00000214 + -2;
    }
    if (iVar7 < 1) {
      iVar7 = 1;
    }
  }
  unaff_s0 = 0;
  for (; uVar4 != 0; uVar4 = uVar4 >> 4) {
    unaff_s1 = unaff_s1 + -1;
    unaff_s0 = unaff_s0 + 1;
    *unaff_s1 = "0123456789abcdef"[uVar4 & 0xf];
  }
  for (; (int)unaff_s0 < iVar7; unaff_s0 = unaff_s0 + 1) {
    unaff_s1 = unaff_s1 + -1;
    *unaff_s1 = 0x30;
  }
  if ((in_stack_00000210 >> 2 & 1) != 0) {
    unaff_s1[-1] = (byte)iVar5;
    unaff_s1[-2] = 0x30;
    iVar5 = SPRINTF_OBJ_7A0();
    return iVar5;
  }
  goto code_r0x800537a4;
}

