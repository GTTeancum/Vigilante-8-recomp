// addr: 0x8004ff50  name: SYS_OBJ_D4C

void SYS_OBJ_D4C(void)

{
  short sVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  uint unaff_s0;
  int iVar11;
  int unaff_s1;
  uint uVar12;
  
  bVar2 = *(short *)(unaff_s1 + 6) < 0x121;
  if (*(char *)(unaff_s1 + 0x12) == '\0') {
    bVar2 = *(short *)(unaff_s1 + 6) < 0x101;
  }
  if (!bVar2) {
    unaff_s0 = unaff_s0 | 0x24;
  }
  SYS_OBJ_20DC(unaff_s0);
  *(undefined1 *)(unaff_s1 + 0x12) = 8;
  if ((((*(short *)(unaff_s1 + 8) != 0) || (*(short *)(unaff_s1 + 10) != 0)) ||
      (*(short *)(unaff_s1 + 0xc) != 0)) ||
     ((*(short *)(unaff_s1 + 0xe) != 0 || (*(char *)(unaff_s1 + 0x12) == '\b')))) {
    cVar3 = FUN_8004f1e8();
    *(char *)(unaff_s1 + 0x12) = cVar3;
    iVar11 = *(short *)(unaff_s1 + 10) + 0x13;
    if (cVar3 == '\0') {
      iVar11 = *(short *)(unaff_s1 + 10) + 0x10;
    }
    uVar12 = iVar11 + *(short *)(unaff_s1 + 0xe);
    if (*(short *)(unaff_s1 + 0xe) == 0) {
      uVar12 = iVar11 + 0xf0;
    }
    sVar1 = *(short *)(unaff_s1 + 4);
    iVar10 = 0;
    if (((0x118 < sVar1) && (iVar10 = 1, 0x160 < sVar1)) &&
       ((iVar10 = 2, 400 < sVar1 && (iVar10 = 4, sVar1 < 0x231)))) {
      iVar10 = 3;
    }
    iVar4 = ((uint)*(byte *)(unaff_s1 + 0x12) * 5 + iVar10) * 4;
    iVar5 = (uint)*(ushort *)(&DAT_800650fa + iVar4) - (uint)*(ushort *)(&DAT_800650f8 + iVar4);
    uVar7 = (uint)*(ushort *)(&DAT_800650f8 + iVar4) +
            (int)*(short *)(unaff_s1 + 8) * (uint)(byte)(&DAT_80065120)[iVar10];
    if (*(short *)(unaff_s1 + 0xc) != 0) {
      iVar5 = iVar5 * *(short *)(unaff_s1 + 0xc) >> 8;
    }
    uVar6 = uVar7 + iVar5;
    if (*(char *)(unaff_s1 + 0x12) == '\0') {
      uVar8 = 500;
      if ((499 < (int)uVar7) && (uVar8 = 0xcb2, (int)uVar7 < 0xcb3)) {
        uVar8 = uVar7;
      }
      uVar7 = uVar8 + (uint)(byte)(&DAT_80065120)[iVar10] * 4;
      if (((int)uVar7 <= (int)uVar6) && (uVar7 = 0xcda, (int)uVar6 < 0xcdb)) {
        uVar7 = uVar6;
      }
      if (iVar11 < 0x10) {
        uVar6 = 0x10;
      }
      else {
        uVar6 = 0x101;
        if (iVar11 < 0x102) {
          SYS_OBJ_FF4(uVar8,iVar11);
          return;
        }
      }
      uVar9 = uVar6 + 2;
      if (((int)(uVar6 + 2) <= (int)uVar12) && (uVar9 = 0x102, (int)uVar12 < 0x103)) {
        uVar9 = uVar12;
      }
    }
    else {
      uVar8 = 0x21c;
      if ((0x21b < (int)uVar7) && (uVar8 = 0xc94, (int)uVar7 < 0xc95)) {
        uVar8 = uVar7;
      }
      uVar7 = uVar8 + (uint)(byte)(&DAT_80065120)[iVar10] * 4;
      if (((int)uVar7 <= (int)uVar6) && (uVar7 = 0xcbc, (int)uVar6 < 0xcbd)) {
        uVar7 = uVar6;
      }
      if (iVar11 < 0x13) {
        uVar6 = 0x13;
      }
      else {
        uVar6 = 0x12f;
        if (iVar11 < 0x130) {
          SYS_OBJ_F64(uVar8,iVar11);
          return;
        }
      }
      uVar9 = uVar6 + 2;
      if ((int)(uVar6 + 2) <= (int)uVar12) {
        SYS_OBJ_100C(uVar8,0x131);
        return;
      }
    }
    SYS_OBJ_20DC((uVar7 & 0xfff) << 0xc | uVar8 & 0xfff | 0x6000000);
    SYS_OBJ_20DC((uVar9 & 0x3ff) << 10 | uVar6 | 0x7000000);
  }
  FUN_80044c44(&DAT_80065090);
  return;
}

