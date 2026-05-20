// addr: 0x800535a8  name: SPRINTF_OBJ_5A4

/* WARNING: Removing unreachable block (ram,0x80053334) */
/* WARNING: Removing unreachable block (ram,0x8005338c) */
/* WARNING: Removing unreachable block (ram,0x800534ac) */
/* WARNING: Removing unreachable block (ram,0x8005377c) */

int SPRINTF_OBJ_5A4(undefined4 param_1,int param_2,undefined4 param_3,char *param_4)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
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
  
code_r0x800535a8:
  uVar4 = *in_stack_00000220;
  in_stack_00000220 = in_stack_00000220 + 1;
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
    *unaff_s1 = param_4[uVar4 & 0xf];
  }
  for (; (int)uVar5 < in_stack_00000218; uVar5 = uVar5 + 1) {
    unaff_s1 = unaff_s1 + -1;
    *unaff_s1 = 0x30;
  }
  if ((in_stack_00000210 >> 2 & 1) != 0) {
    unaff_s1[-1] = (byte)param_2;
    unaff_s1[-2] = 0x30;
    iVar2 = SPRINTF_OBJ_7A0();
    return iVar2;
  }
code_r0x800537a4:
  iVar2 = unaff_s3 + unaff_s2;
  if (((int)uVar5 < in_stack_00000214) && ((in_stack_00000210 & 1) == 0)) {
    do {
      *(char *)(unaff_s3 + unaff_s2) = (char)unaff_s4;
      in_stack_00000214 = in_stack_00000214 + -1;
      unaff_s2 = unaff_s2 + 1;
    } while ((int)uVar5 < in_stack_00000214);
    iVar2 = unaff_s3 + unaff_s2;
  }
  FUN_80044d9c(iVar2,unaff_s1,uVar5);
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
    iVar2 = SPRINTF_OBJ_82C();
    return iVar2;
  }
  in_stack_00000210 = 0;
  in_stack_00000214 = 0;
  in_stack_00000218 = 0;
  iVar2 = in_stack_0000024c + 1;
  while( true ) {
    while( true ) {
      while( true ) {
        while( true ) {
          while( true ) {
            iVar1 = iVar2 + 1;
            param_2 = (int)*(char *)(iVar2 + 1);
            if (param_2 != unaff_s5) break;
            in_stack_00000210 = in_stack_00000210 | 1;
            iVar2 = iVar1;
          }
          if (param_2 != unaff_s6) break;
          in_stack_00000210 = in_stack_00000210 | 2;
          iVar2 = iVar1;
        }
        if (param_2 != unaff_s4) break;
        in_stack_00000210 = (uint)CONCAT11(*(char *)(iVar2 + 1),(byte)in_stack_00000210);
        iVar2 = iVar1;
      }
      if (param_2 != 0x23) break;
      in_stack_00000210 = in_stack_00000210 | 4;
      iVar2 = iVar1;
    }
    if (param_2 != 0x30) break;
    in_stack_00000210 = in_stack_00000210 | 8;
    iVar2 = iVar1;
  }
  if (param_2 == 0x2a) {
    iVar2 = SPRINTF_OBJ_1BC(*in_stack_00000220,(int)*(char *)(iVar2 + 2));
    return iVar2;
  }
  while (param_2 - 0x30U < 10) {
    in_stack_00000214 = in_stack_00000214 * 10 + -0x30 + param_2;
    param_2 = (int)*(char *)(iVar1 + 1);
    iVar1 = iVar1 + 1;
  }
  iVar2 = 0x23;
  in_stack_0000024c = iVar1;
  if (param_2 == 0x2e) {
    param_2 = (int)*(char *)(iVar1 + 1);
    iVar2 = iVar1;
    if (param_2 == 0x2a) {
      iVar2 = SPRINTF_OBJ_258(iVar1,(int)*(char *)(iVar1 + 2));
      return iVar2;
    }
    while (in_stack_0000024c = iVar2 + 1, param_2 - 0x30U < 10) {
      in_stack_00000218 = in_stack_00000218 * 10 + -0x30 + param_2;
      param_2 = (int)*(char *)(iVar2 + 2);
      iVar2 = in_stack_0000024c;
    }
    iVar2 = iVar1;
    if (-1 < in_stack_00000218) {
      in_stack_00000210 = in_stack_00000210 | 0x10;
    }
  }
  unaff_s1 = &stack0x00000210;
  if ((in_stack_00000210 & 1) != 0) {
    in_stack_00000210 = in_stack_00000210 & 0xfffffff7;
  }
  switch(param_2) {
  case 0x4c:
    iVar2 = SPRINTF_OBJ_29C(iVar2,(int)*(char *)(in_stack_0000024c + 1));
    return iVar2;
  default:
    if (param_2 != 0x25) goto SPRINTF_OBJ_84C;
    goto SPRINTF_OBJ_794;
  case 99:
    iVar2 = SPRINTF_OBJ_7A0();
    return iVar2;
  case 100:
  case 0x69:
    uVar4 = *in_stack_00000220;
    if ((int)uVar4 < 0) {
      iVar2 = SPRINTF_OBJ_390(-uVar4);
      return iVar2;
    }
    if ((in_stack_00000210 >> 1 & 1) != 0) {
      iVar2 = SPRINTF_OBJ_390();
      return iVar2;
    }
    break;
  case 0x68:
    iVar2 = SPRINTF_OBJ_2E4();
    return iVar2;
  case 0x6c:
    iVar2 = SPRINTF_OBJ_2E4();
    return iVar2;
  case 0x6e:
    *(int *)*in_stack_00000220 = unaff_s2;
    iVar2 = SPRINTF_OBJ_82C();
    return iVar2;
  case 0x6f:
    uVar4 = *in_stack_00000220;
    in_stack_00000220 = in_stack_00000220 + 1;
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
      iVar2 = SPRINTF_OBJ_7A0();
      return iVar2;
    }
    goto code_r0x800537a4;
  case 0x70:
  case 0x58:
    iVar2 = SPRINTF_OBJ_5A4();
    return iVar2;
  case 0x73:
    goto SPRINTF_OBJ_6C8;
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
    if (((in_stack_00000210 >> 3 & 1) != 0) && (in_stack_00000218 = in_stack_00000214, bVar6 != 0))
    {
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
    iVar2 = SPRINTF_OBJ_7A0();
    return iVar2;
  }
  goto code_r0x800537a4;
SPRINTF_OBJ_59C:
  param_4 = "0123456789abcdef";
  goto code_r0x800535a8;
SPRINTF_OBJ_6C8:
  unaff_s1 = (byte *)*in_stack_00000220;
  in_stack_00000220 = in_stack_00000220 + 1;
  if ((in_stack_00000210 >> 2 & 1) == 0) {
    if ((in_stack_00000210 >> 4 & 1) == 0) {
      FUN_80052544(unaff_s1);
      iVar2 = SPRINTF_OBJ_7A0();
      return iVar2;
    }
    pvVar3 = memchr(unaff_s1,'\0',in_stack_00000218);
    uVar5 = (int)pvVar3 - (int)unaff_s1;
    if (pvVar3 == (void *)0x0) {
      iVar2 = SPRINTF_OBJ_7A0();
      return iVar2;
    }
  }
  else {
    uVar5 = (uint)*unaff_s1;
    unaff_s1 = unaff_s1 + 1;
    if (((in_stack_00000210 >> 4 & 1) != 0) && (in_stack_00000218 < (int)uVar5)) {
      iVar2 = SPRINTF_OBJ_7A0();
      return iVar2;
    }
  }
  goto code_r0x800537a4;
}

