// addr: 0x80051440  name: SYS_OBJ_223C

uint SYS_OBJ_223C(void)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *unaff_s0;
  int unaff_s1;
  undefined4 unaff_s2;
  code *unaff_s3;
  
  while ((DAT_8006514c + 1 & 0x3f) == DAT_80065150) {
    iVar5 = SYS_OBJ_29C0();
    if (iVar5 != 0) {
      return 0xffffffff;
    }
    SYS_OBJ_24A0();
  }
  DAT_80065154 = SetIntrMask(0);
  DAT_8006502c = 1;
  if ((DAT_80065025 != '\0') &&
     (((DAT_8006514c != DAT_80065150 || ((DMA_GPU_CHCR & 0x1000000) != 0)) || (DAT_80065030 != 0))))
  {
    DMACallback(2,SYS_OBJ_24A0);
    iVar5 = 0;
    if (unaff_s1 == 0) {
      *(undefined4 **)(&DAT_800a344c + DAT_8006514c * 0x60) = unaff_s0;
      *(undefined4 *)(&DAT_800a3450 + DAT_8006514c * 0x60) = unaff_s2;
      *(code **)(&DAT_800a3448 + DAT_8006514c * 0x60) = unaff_s3;
      DAT_8006514c = DAT_8006514c + 1 & 0x3f;
      SetIntrMask(DAT_80065154);
      SYS_OBJ_24A0();
      return DAT_8006514c - DAT_80065150 & 0x3f;
    }
    while( true ) {
      iVar2 = unaff_s1;
      if (unaff_s1 < 0) {
        iVar2 = unaff_s1 + 3;
      }
      iVar3 = iVar5 * 4;
      if (iVar2 >> 2 <= iVar5) break;
      uVar4 = *unaff_s0;
      unaff_s0 = unaff_s0 + 1;
      iVar5 = iVar5 + 1;
      *(undefined4 *)(&DAT_800a3454 + iVar3 + DAT_8006514c * 0x60) = uVar4;
    }
    *(undefined **)(&DAT_800a344c + DAT_8006514c * 0x60) = &DAT_800a3454 + DAT_8006514c * 0x60;
    uVar1 = SYS_OBJ_23F4();
    return uVar1;
  }
  do {
  } while ((GPU_REG1 & 0x4000000) == 0);
  (*unaff_s3)();
  SetIntrMask(DAT_80065154);
  uVar1 = SYS_OBJ_2484();
  return uVar1;
}

