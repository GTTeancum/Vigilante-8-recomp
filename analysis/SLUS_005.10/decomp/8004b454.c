// addr: 0x8004b454  name: CDREAD_OBJ_740

int CDREAD_OBJ_740(void)

{
  int iVar1;
  int unaff_s0;
  int *unaff_s1;
  int unaff_s2;
  u_char *unaff_s3;
  
  while( true ) {
    do {
      if ((unaff_s2 != 0) || (unaff_s0 < 1)) {
        CdReady(1,unaff_s3);
        return unaff_s0;
      }
      iVar1 = VSync(-1);
      unaff_s0 = -1;
    } while (*unaff_s1 + 0x4b0 < iVar1);
    if ((unaff_s1[-2] < 0) || (iVar1 = VSync(-1), unaff_s1[-1] + 0x3c < iVar1)) break;
    unaff_s0 = unaff_s1[-2];
  }
  CDREAD_OBJ_32C(1);
  iVar1 = CDREAD_OBJ_740();
  return iVar1;
}

