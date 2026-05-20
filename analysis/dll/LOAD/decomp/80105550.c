// addr: 0x80105550  name: FUN_80105550

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80105550(ushort *param_1)

{
  ushort *puVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  ushort uVar5;
  ushort uVar6;
  short sVar7;
  char *pcVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  ushort *puVar13;
  ushort *puVar14;
  short *psVar15;
  
  iVar12 = 0;
  do {
    iVar11 = 0;
    iVar9 = iVar12 << 3;
    do {
      pcVar8 = &DAT_80106dbc + iVar9;
      if (*pcVar8 != '\0') {
        iVar9 = 1;
      }
      *pcVar8 = '\0';
      iVar10 = iVar9;
      if ((&DAT_80106dbd)[iVar9] != '\0') {
        iVar10 = 1;
      }
      (&DAT_80106dbd)[iVar9] = 0;
      iVar11 = iVar11 + 1;
      iVar9 = iVar10 + 2;
    } while (iVar11 < 4);
    iVar12 = iVar12 + 1;
  } while (iVar12 < 8);
  puVar14 = param_1 + 3;
  iVar12 = 0;
  psVar15 = (short *)&DAT_8008f020;
  do {
    uVar2 = *puVar14;
    uVar3 = puVar14[-2];
    iVar9 = (uVar2 >> 8 & 7) * 8;
    uVar4 = uVar3 >> 8;
    uVar6 = uVar4 & 0x7f;
    uVar5 = puVar14[-1] >> 8;
    *psVar15 = _DAT_80065b4e +
               ((byte)(&DAT_80106dbc)[iVar9] + uVar6 |
               ((byte)(&DAT_80106dbd)[iVar9] + uVar5) * 0x100);
    psVar15[2] = _DAT_80065b4e +
                 ((byte)(&DAT_80106dbe)[iVar9] + uVar6 |
                 ((byte)(&DAT_80106dbf)[iVar9] + uVar5) * 0x100);
    psVar15[4] = _DAT_80065b4e +
                 ((byte)(&DAT_80106dc0)[iVar9] + uVar6 |
                 ((byte)(&DAT_80106dc1)[iVar9] + uVar5) * 0x100);
    psVar15[6] = _DAT_80065b4e +
                 ((byte)(&DAT_80106dc2)[iVar9] + uVar6 |
                 ((byte)(&DAT_80106dc3)[iVar9] + uVar5) * 0x100);
    sVar7 = 0;
    if ((*param_1 & 0x1000) == 0) {
      sVar7 = GetTPage/*0x80052214*/(*_DAT_80065afc,0,
                              (int)*(short *)_DAT_80065afc[3] +
                              ((((uint)uVar4 | (uVar3 & 0xff) << 8) >> 7) <<
                              (*_DAT_80065afc & 3) + 5),(int)((short *)_DAT_80065afc[3])[1]);
    }
    iVar11 = 0;
    psVar15[7] = sVar7;
    psVar15[5] = sVar7;
    psVar15[3] = sVar7;
    psVar15[1] = sVar7;
    psVar15[0xf] = uVar2 >> 0xb & 1;
    iVar9 = iVar12;
    puVar13 = param_1;
    do {
      puVar1 = puVar13 + 4;
      puVar13 = puVar13 + 1;
      iVar11 = iVar11 + 1;
      *(ushort *)(iVar9 + -0x7ff70fd0) = *puVar1 >> 8 | *puVar1 << 8;
      iVar9 = iVar9 + 2;
    } while (iVar11 < 7);
    iVar12 = iVar12 + 0x20;
    psVar15 = psVar15 + 0x10;
    puVar14 = puVar14 + 0x14;
    param_1 = param_1 + 0x14;
  } while ((int)psVar15 < -0x7ff6efe0);
  return;
}

