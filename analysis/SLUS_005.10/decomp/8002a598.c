// addr: 0x8002a598  name: FUN_8002a598

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: ram */

void FUN_8002a598(void)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  undefined *puVar4;
  ushort *puVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  undefined4 *puVar14;
  undefined4 *puVar15;
  undefined1 auStack_50 [7];
  undefined1 uStack_49;
  ushort local_48;
  undefined1 auStack_40 [8];
  ushort local_38;
  uint local_30;
  int local_2c;
  int local_28 [2];
  
  _DAT_800a2858 = CONCAT13(DAT_800a285b,CONCAT12(DAT_800a285a,CONCAT11(DAT_800a2859,DAT_800a2858)));
  iVar3 = FUN_80015f80("HUD\\HUD.TBL");
  iVar11 = 0;
  puVar6 = &DAT_800a28a4;
  puVar10 = &DAT_800a28ac;
  iVar13 = iVar3;
  do {
    FUN_800187e4(iVar3 + *(int *)(iVar13 + 4),auStack_50);
    FUN_80018c3c(puVar6,auStack_50);
    iVar13 = iVar13 + 4;
    iVar11 = iVar11 + 1;
    puVar6 = puVar6 + 7;
    *(byte *)((int)puVar10 + 7) = *(byte *)((int)puVar10 + 7) | 2;
    puVar10 = puVar10 + 7;
  } while (iVar11 < 4);
  puVar6 = &DAT_800a28a0;
  puVar10 = &DAT_800a2914;
  do {
    uVar7 = puVar6[1];
    uVar8 = puVar6[2];
    uVar9 = puVar6[3];
    *puVar10 = *puVar6;
    puVar10[1] = uVar7;
    puVar10[2] = uVar8;
    puVar10[3] = uVar9;
    puVar6 = puVar6 + 4;
    puVar10 = puVar10 + 4;
  } while (puVar6 != (undefined4 *)&UNK_800a2910);
  *puVar10 = _UNK_800a2910;
  FUN_80018d64(&DAT_800a28a0,4);
  FUN_80018d64(&DAT_800a2914,4);
  FUN_800187e4(iVar3 + *(int *)(iVar3 + 0x34),&DAT_80065ba8);
  uRam000008ac = uRam000008ac | 0x20;
  FUN_80018c3c(&DAT_800a2fb8,&DAT_80065ba8);
                    /* WARNING: Read-only address (ram,0x800a2fd2) is written */
                    /* WARNING: Read-only address (ram,0x800a2fd0) is written */
  DAT_800a2fd0 = CONCAT22(0x10,(undefined2)DAT_800a2fd0);
                    /* WARNING: Read-only address (ram,0x800a2fc7) is written */
                    /* WARNING: Read-only address (ram,0x800a2fc4) is written */
  DAT_800a2fc4 = DAT_800a2fc4 | 0x2000000;
  DAT_800a2fcc = CONCAT31(DAT_800a2fcc._1_3_,uRam000008aa);
  DAT_800a2fd4 = DAT_800a2fb8;
  DAT_800a2fd8 = DAT_800a2fbc;
  DAT_800a2fdc = DAT_800a2fc0;
  DAT_800a2fe0 = DAT_800a2fc4;
  DAT_800a2fe4 = DAT_800a2fc8;
  DAT_800a2fe8 = DAT_800a2fcc;
  DAT_800a2fec = DAT_800a2fd0;
  FUN_800187e4(iVar3 + *(int *)(iVar3 + 0x38),&DAT_80065b98);
  iVar13 = 0;
  puVar6 = &DAT_800a2988;
  puVar10 = &DAT_800a2990;
  uRam0000089c = uRam0000089c | 0x20;
  do {
    FUN_80018c3c(puVar6,&DAT_80065b98);
    *(byte *)((int)puVar10 + 7) = *(byte *)((int)puVar10 + 7) | 2;
    cVar2 = (char)iVar13;
    iVar13 = iVar13 + 1;
    puVar10 = puVar10 + 0xe;
    *(undefined2 *)((int)puVar6 + 0x1a) = 0x10;
    *(char *)((int)puVar6 + 0x15) = *(char *)((int)puVar6 + 0x15) + cVar2 * '\x10';
    puVar6[7] = *puVar6;
    puVar6[8] = puVar6[1];
    puVar6[9] = puVar6[2];
    puVar6[10] = puVar6[3];
    puVar6[0xb] = puVar6[4];
    puVar6[0xc] = puVar6[5];
    puVar6[0xd] = puVar6[6];
    puVar6 = puVar6 + 0xe;
  } while (iVar13 < 3);
  uVar12 = 0;
  puVar4 = &UNK_800a2bb8;
  puVar6 = &DAT_800a2bc4;
  do {
    puVar4[3] = 3;
    puVar4[7] = 0x60;
    *puVar6 = 0x20002;
    puVar6 = puVar6 + 4;
    uVar12 = uVar12 + 1;
    puVar4 = puVar4 + 0x10;
  } while (uVar12 < 0x40);
  FUN_800187e4(iVar3 + *(int *)(iVar3 + 0x1c),auStack_50);
  FUN_800187e4(iVar3 + *(int *)(iVar3 + 0x18),auStack_40);
  iVar13 = 0;
  puVar6 = &DAT_800a2a30;
  puVar10 = &DAT_800a2ad8;
  puVar15 = &DAT_800a2a38;
  puVar14 = &DAT_800a2ae0;
  local_48 = local_48 | 0x20;
  local_38 = local_38 | 0x20;
  do {
    FUN_80018c3c(puVar10,auStack_50);
    FUN_80018c3c(puVar6,auStack_40);
    *(byte *)((int)puVar14 + 7) = *(byte *)((int)puVar14 + 7) | 2;
    iVar13 = iVar13 + 1;
    puVar14 = puVar14 + 0xe;
    *(byte *)((int)puVar15 + 7) = *(byte *)((int)puVar15 + 7) | 2;
    puVar10[7] = *puVar10;
    puVar10[8] = puVar10[1];
    puVar10[9] = puVar10[2];
    puVar10[10] = puVar10[3];
    puVar10[0xb] = puVar10[4];
    puVar10[0xc] = puVar10[5];
    puVar10[0xd] = puVar10[6];
    puVar6[7] = *puVar6;
    puVar6[8] = puVar6[1];
    puVar6[9] = puVar6[2];
    puVar6[10] = puVar6[3];
    puVar6[0xb] = puVar6[4];
    puVar6[0xc] = puVar6[5];
    puVar6[0xd] = puVar6[6];
    puVar6 = puVar6 + 0xe;
    puVar10 = puVar10 + 0xe;
    puVar15 = puVar15 + 0xe;
  } while (iVar13 < 3);
  FUN_800187e4(iVar3 + *(int *)(iVar3 + 0x20),auStack_50);
  local_48 = local_48 | 0x20;
  FUN_80018c3c(&DAT_800a2b80,auStack_50);
                    /* WARNING: Read-only address (ram,0x800a2b8f) is written */
                    /* WARNING: Read-only address (ram,0x800a2b8c) is written */
  DAT_800a2b8c = DAT_800a2b8c | 0x2000000;
  DAT_800a2b9c = DAT_800a2b80;
  DAT_800a2ba0 = DAT_800a2b84;
  DAT_800a2ba4 = DAT_800a2b88;
  DAT_800a2ba8 = DAT_800a2b8c;
  DAT_800a2bac = DAT_800a2b90;
  DAT_800a2bb0 = DAT_800a2b94;
  DAT_800a2bb4 = DAT_800a2b98;
  uRam00000880 = uStack_49;
  FUN_80045088(iVar3);
  FUN_80018bd0(&DAT_800a2828);
                    /* WARNING: Read-only address (ram,0x800a2847) is written */
  DAT_800a2844._3_1_ = 3;
  DAT_800a2848._3_1_ = 0x60;
  DAT_800a2857 = 3;
  DAT_800a2848._0_1_ = 0xff;
  DAT_800a2848._1_1_ = 0;
  DAT_800a2848._2_1_ = 0;
  _DAT_800a2858 = 0x6000ff00;
  DAT_800a2862 = 2;
                    /* WARNING: Read-only address (ram,0x800a2852) is written */
  DAT_800a2850._2_2_ = 2;
  DAT_800a2834._3_1_ = DAT_800a2834._3_1_ | 2;
                    /* WARNING: Read-only address (ram,0x800a2837) is written */
  MargePrim(&DAT_800a2844,&DAT_800a2854);
  MargePrim(&DAT_800a2828,&DAT_800a2844);
  puVar6 = &DAT_800a2864;
  puVar10 = &DAT_800a2828;
  do {
    puVar15 = puVar10;
    puVar14 = puVar6;
    uVar7 = puVar15[1];
    uVar8 = puVar15[2];
    uVar9 = puVar15[3];
    *puVar14 = *puVar15;
    puVar14[1] = uVar7;
    puVar14[2] = uVar8;
    puVar14[3] = uVar9;
    puVar6 = puVar14 + 4;
    puVar10 = puVar15 + 4;
  } while (puVar15 + 4 != (undefined4 *)&DAT_800a2858);
  uVar12 = 0;
  uVar7 = puVar15[5];
  uVar8 = puVar15[6];
  puVar14[4] = _DAT_800a2858;
  puVar14[5] = uVar7;
  puVar14[6] = uVar8;
  uRam0000085f = 1;
  uRam00000860 = 0xe1000400;
  uRam00000867 = 1;
  uRam00000868 = 0xe1000400;
  do {
    puVar5 = (ushort *)FUN_8001bda0(DAT_800737d4,(&DAT_8005ea5c)[uVar12]);
    (&DAT_80065b70)[uVar12] = puVar5;
    uVar12 = uVar12 + 1;
    *puVar5 = *puVar5 | 2;
  } while (uVar12 < 4);
  uVar7 = FUN_80015f80("HUD\\Font.FNT");
  uRam0000087c = FUN_80019034(uVar7,0x22);
  iVar13 = -1;
  FUN_8001910c();
  FUN_800159b4("HUD\\Arms.exp");
  FUN_800225d4(&local_30,local_28);
  local_28[0] = local_2c;
joined_r0x8002ab48:
  do {
    if (local_28[0] == 0) {
LAB_8002aca0:
      FUN_80015a00();
      return;
    }
    iVar3 = FUN_800225d4(&local_30,local_28);
    if (iVar3 != 0) {
      FUN_80045088(iVar3);
      goto joined_r0x8002ab48;
    }
    if ((local_30 >> 0x18 | local_30 >> 8 & 0xff00 | (local_30 & 0xff00) << 8 | local_30 << 0x18) ==
        0x584f4246) {
      iVar13 = iVar13 + 1;
      if (iVar13 == DAT_80065674) {
        puVar6 = (undefined4 *)FUN_8002263c(local_2c,1);
        DAT_800737e0 = puVar6;
        if (cRam00000015 < '\x03') {
          uRam000008b0 = FUN_8001bda0(puVar6,0);
          goto LAB_8002aca0;
        }
        FUN_80045088(*puVar6);
        bVar1 = '\x02' < cRam00000015;
        *puVar6 = 0;
        if ((bVar1) && (iVar13 == DAT_80065675)) {
          DAT_800737e4 = puVar6;
        }
        goto joined_r0x8002ab48;
      }
      if (('\x02' < cRam00000015) && (iVar13 == DAT_80065675)) {
        puVar6 = (undefined4 *)FUN_8002263c(local_2c,1);
        DAT_800737e4 = puVar6;
        FUN_80045088(*puVar6);
        *puVar6 = 0;
        goto joined_r0x8002ab48;
      }
    }
    FUN_80015bf0(local_2c,1);
  } while( true );
}

