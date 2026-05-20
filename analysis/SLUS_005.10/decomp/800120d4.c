// addr: 0x800120d4  name: FUN_800120d4

void FUN_800120d4(void)

{
  short *psVar1;
  bool bVar2;
  ushort uVar3;
  short sVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  uint uVar9;
  char *pcVar10;
  short *psVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  
  psVar11 = psRam00000610;
  if (bRam00000601 != bRam00000614) {
    bRam00000614 = bRam00000614 + 1 & 7;
  }
  iVar8 = (uint)bRam00000614 * 0x44;
  pcVar10 = &DAT_8006ecb8 + iVar8;
  uVar9 = 0xffff;
  if (*pcVar10 != -1) {
    uVar9 = (uint)CONCAT11((&DAT_8006ecba)[iVar8],(&DAT_8006ecbb)[iVar8]);
  }
  if ((&DAT_8006ecda)[iVar8] == -1) {
    uVar5 = 0xffff0000;
  }
  else {
    uVar5 = (uint)CONCAT11((&DAT_8006ecdc)[iVar8],(&DAT_8006ecdd)[iVar8]) << 0x10;
  }
  uVar9 = ~(uVar5 | uVar9);
  if ((&DAT_8006ecb9)[iVar8] == '#') {
    uVar5 = (uint)((&DAT_8006ecbd)[iVar8] == -1) << 6;
    if ((&DAT_8006ecbf)[iVar8] == -1) {
      uVar5 = uVar5 | 4;
    }
    if ((&DAT_8006ecbe)[iVar8] == -1) {
      uVar9 = uVar9 | 0x80 | uVar5;
    }
    else {
      uVar9 = uVar9 | uVar5;
    }
  }
  if ((&DAT_8006ecdb)[iVar8] == '#') {
    uVar5 = (uint)((&DAT_8006ecdf)[iVar8] == -1) << 0x16;
    if ((&DAT_8006ece1)[iVar8] == -1) {
      uVar5 = uVar5 | 0x40000;
    }
    if ((&DAT_8006ece0)[iVar8] == -1) {
      uVar9 = uVar9 | 0x800000 | uVar5;
    }
    else {
      uVar9 = uVar9 | uVar5;
    }
  }
  if (iRam00000618 == 1) {
    sRam0000061e = sRam0000061e + -1;
    if (sRam0000061e == -1) {
      sRam0000061e = psRam00000610[2];
      psRam00000610 = psRam00000610 + 2;
    }
    bVar2 = uVar9 != 0;
    uVar9 = (uint)(ushort)psRam00000610[1];
    if (bVar2) {
      uVar9 = (ushort)psRam00000610[1] | 0x800;
    }
  }
  else if (iRam00000618 == 2) {
    if ((psRam00000610 == (short *)(iRam00000634 + 8)) ||
       ((uint)(ushort)psRam00000610[1] != (uVar9 & 0xffff))) {
      psVar1 = psRam00000610 + 3;
      psRam00000610 = psRam00000610 + 2;
      *psVar1 = (short)uVar9;
      psVar11[2] = 0;
    }
    else {
      *psRam00000610 = *psRam00000610 + 1;
    }
  }
  uVar5 = 0;
  iVar8 = 0;
  iVar14 = 0;
  iVar12 = 0;
  iVar13 = 0;
  uRam0000062c = uVar9 & 0xffff | (uVar9 & ~uRam000005d4) << 0x10;
  uRam00000630 = uVar9 >> 0x10 | uVar9 & ~uRam000005d4 & 0xffff0000;
  uRam000005d4 = uVar9;
  do {
    psVar11 = (short *)((int)&DAT_80065c28 + iVar13);
    if (iRam00000618 == 0) {
      sVar4 = FUN_80011f8c(uVar5);
    }
    else {
      sVar4 = 2;
    }
    *psVar11 = sVar4;
    *(undefined4 *)((int)&DAT_80065c34 + iVar13) = (&DAT_80065930)[uVar5];
    (&DAT_80065943)[uVar5 * 8] = (byte)(*(uint *)(&DAT_80056778 + *psVar11 * 8 + iVar12) >> 0xc) & 1
    ;
    iVar6 = *psVar11 * 8 + iVar12;
    uVar7 = FUN_80012088((&DAT_80065930)[uVar5] & 0xf7fff7ff,&DAT_80065940 + uVar5 * 8,
                         *(undefined4 *)(&DAT_80056774 + iVar6),
                         *(undefined4 *)(&DAT_80056778 + iVar6));
    *(undefined4 *)((int)&DAT_80065c30 + iVar13) = uVar7;
    uVar9 = *(uint *)((int)&DAT_80065c34 + iVar13) & 0xf0000000;
    if (uVar9 == 0) {
      sVar4 = *(short *)((int)&DAT_80065c2a + iVar13);
      if ((*(short *)((int)&DAT_80065c2a + iVar13) != 0) &&
         (*(short *)((int)&DAT_80065c2a + iVar13) = sVar4 + -1, sVar4 == 1)) {
        *(undefined4 *)((int)&DAT_80065c2c + iVar13) = 0;
      }
    }
    else {
      gte_ldLZCS(uVar9 >> 1);
      uVar9 = gte_stLZCR();
      *(uint *)((int)&DAT_80065c2c + iVar13) = *(int *)((int)&DAT_80065c2c + iVar13) << 4 | uVar9;
      *(undefined2 *)((int)&DAT_80065c2a + iVar13) = 0x14;
    }
    if (2 < *psVar11) {
      uVar3 = *(ushort *)(&DAT_800567d4 + *psVar11 * 2 + iVar14 * 4);
      uVar7 = *(undefined4 *)((int)&DAT_80065c38 + iVar13);
      *(undefined4 *)((int)&DAT_80065c38 + iVar13) = 0x80808080;
      *(undefined4 *)((int)&DAT_80065c3c + iVar13) = uVar7;
      if ((uVar3 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 & 0xf) + 0xf) =
             (&DAT_80065c58)[(uint)(byte)pcVar10[4] + iVar8];
      }
      if ((uVar3 >> 4 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 4 & 0xf) + 0xf) =
             (&DAT_80065d58)[(uint)(byte)pcVar10[5] + iVar8];
      }
      if ((uVar3 >> 8 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 8 & 0xf) + 0xf) =
             (&DAT_80065e58)[(uint)(byte)pcVar10[6] + iVar8];
      }
      if (uVar3 >> 0xc != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 0xc) + 0xf) =
             (&DAT_80065f58)[(uint)(byte)pcVar10[7] + iVar8];
      }
    }
    iVar8 = iVar8 + 0x400;
    pcVar10 = pcVar10 + 0x22;
    iVar14 = iVar14 + 3;
    iVar12 = iVar12 + 0x30;
    uVar5 = uVar5 + 1;
    iVar13 = iVar13 + 0x18;
  } while (uVar5 < 2);
  return;
}

