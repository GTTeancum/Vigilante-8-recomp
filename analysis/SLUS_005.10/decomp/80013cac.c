// addr: 0x80013cac  name: FUN_80013cac

/* WARNING: Removing unreachable block (ram,0x80014340) */

void FUN_80013cac(void)

{
  short sVar1;
  bool bVar2;
  u_long uVar3;
  int iVar4;
  char *pcVar5;
  undefined4 uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint *puVar11;
  undefined1 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  uint uVar15;
  uint uVar16;
  uint *puVar17;
  int iVar18;
  int iVar19;
  undefined4 uStackY_f0;
  char local_d0 [64];
  char local_90 [64];
  RECT local_50;
  u_long local_48;
  uint local_44;
  uint local_40;
  uint local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  
  local_38 = 0;
  pcVar5 = local_d0;
  do {
    pcVar5[3] = '\x03';
    pcVar5[7] = '`';
    pcVar5[4] = '\0';
    pcVar5[5] = '\0';
    pcVar5[6] = '\0';
    if (pcVar5 == local_d0) {
      pcVar5[8] = '\0';
      pcVar5[9] = '\0';
      pcVar5[10] = 'w';
      pcVar5[0xb] = '\0';
      pcVar5[0xc] = '@';
      pcVar5[0xd] = '\x01';
      pcVar5[0xe] = '\x02';
      pcVar5[0xf] = '\0';
    }
    else {
      pcVar5[8] = -0x61;
      pcVar5[9] = '\0';
      pcVar5[10] = '\0';
      pcVar5[0xb] = '\0';
      pcVar5[0xc] = '\x02';
      pcVar5[0xd] = '\0';
      pcVar5[0xe] = -0x10;
      pcVar5[0xf] = '\0';
    }
    *(undefined4 *)(pcVar5 + 0x10) = *(undefined4 *)pcVar5;
    *(undefined4 *)(pcVar5 + 0x14) = *(undefined4 *)(pcVar5 + 4);
    *(undefined4 *)(pcVar5 + 0x18) = *(undefined4 *)(pcVar5 + 8);
    *(undefined4 *)(pcVar5 + 0x1c) = *(undefined4 *)(pcVar5 + 0xc);
    pcVar5 = pcVar5 + 0x20;
  } while (pcVar5 < local_90);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f208,0,0,0x140,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f264,0,0xf0,0x140,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f2c0,0,0,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f31c,0,0xf0,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f378,0,0x79,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f3d4,0,0x169,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f430,0,0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f48c,0,0xf0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f4e8,0xa1,0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f544,0xa1,0xf0,0x9f,0xf0);
  SetDefDispEnv((DISPENV *)&DAT_8006f5a0,0,0xf0,0x140,0xf0);
  SetDefDispEnv((DISPENV *)&DAT_8006f5b4,0,0,0x140,0xf0);
  DAT_8006eca0 = &DAT_8006eca4;
  DAT_8006eca4 = 0;
  DAT_8006eca8 = &DAT_8006eca0;
  DAT_8006ecac = &DAT_8006ecb0;
  DAT_8006ecb0 = 0;
  DAT_8006ecb4 = &DAT_8006ecac;
  FUN_80043ef0();
  DAT_8006eed8 = CONCAT13(1,(undefined3)DAT_8006eed8);
  DAT_8006eedc = 0xe1000220;
  DAT_8006eee3 = 3;
  DAT_8006eee4 = CONCAT13(0x62,(undefined3)DAT_8006eee4);
  DAT_8006eee8 = 0;
  DAT_8006eeec = 0x140;
  DAT_8006eeee = 0xf0;
  MargePrim(&DAT_8006eed8,&DAT_8006eee0);
  do {
    iVar19 = 0;
    FUN_80011834();
    local_34 = 0x78;
    FUN_800128d4();
    FUN_800251fc(0x40);
    if (local_38 == 0) {
      iVar4 = FUN_80011adc("Shell\\Shell.dll");
      pcVar5 = (char *)(**(code **)(iVar4 + 4))();
      if (pcVar5 == (char *)0x0) {
        pcVar5 = "";
      }
      strcpy(local_90,pcVar5);
      FUN_80045088(iVar4);
      if (local_90[0] == '\0') {
        return;
      }
    }
    uVar15 = 0;
    local_38 = 0;
    DAT_8006f5a8 = (short)cRam00000018;
    DAT_8006f5aa = (short)cRam00000019;
    DAT_8006f5bc = DAT_8006f5a8;
    DAT_8006f5be = DAT_8006f5aa;
    FUN_80029dec();
    uVar8 = 0xe000;
    bVar2 = true;
    do {
      if ((bVar2) || ((&UNK_8006567a)[uVar15] != '\0')) {
        uVar8 = uVar8 | 1 << ((int)(&DAT_80065674)[uVar15] & 0x1fU);
      }
      uVar15 = uVar15 + 1;
      bVar2 = uVar15 < 2;
    } while (uVar15 < 8);
    FUN_800227a4(uVar8);
    uRam000005f8 = FUN_80044360("Sounds\\Main.SND");
    uVar6 = FUN_80015f80("Misc\\Game.FNT");
    FUN_800165cc(0);
    FUN_80017fd4(1);
    iRam00000628 = FUN_80019034(uVar6,0x23);
    FUN_8001910c();
    FUN_8002a598();
    if (bRam00000015 == 5) {
      FUN_8001714c(0xbb40e64d);
    }
    DAT_8006f100 = 0;
    DAT_8006eff8 = 0;
    DAT_8006eef0 = 0;
    uVar8 = (uint)(bRam00000015 - 3 < 2);
    iRam00000010 = uVar8 << 1;
    uRam000006cf = 0xff;
    if (uVar8 != 0) {
      uRam000006cf = 0xab;
    }
    iVar18 = 1;
    iVar4 = 1;
    if (bRam00000015 == 0) {
      iVar7 = *(int *)(iRam00000608 + DAT_80065674 * 8 + 8) + cRam00000600 * 0x10;
      pcVar5 = *(char **)(iVar7 + 0xc);
      uRam000006f0 = *(undefined2 *)(iVar7 + 2);
      uRam000007dc = *(undefined2 *)(iVar7 + 4);
      uVar12 = *(undefined1 *)(iVar7 + 1);
    }
    else {
      if (iRam00000618 == 0) {
        pcVar5 = "";
      }
      else {
        pcVar5 = "Loading Demo Level";
      }
      uVar12 = 0;
    }
    FUN_80022ba8(local_90,pcVar5,uVar12);
    FUN_80012980();
    uRam0000000c = 0;
    FUN_800212c4(0);
    if (bRam00000015 == 0) {
      FUN_8001a0ac(&DAT_80065604,0);
      FUN_80019960(iRam00000628,&DAT_800102b0,0x10,0xc0);
      do {
        FUN_800126f0();
      } while ((uRam0000062c & 0x40) == 0);
    }
    local_3c = 0;
    local_40 = puRam000007d0[0x38];
    if (puRam000007d4 != (uint *)0x0) {
      local_3c = puRam000007d4[0x38];
    }
    local_50.w = 0x140;
    local_50.x = 0;
    local_50.y = 0;
    local_50.h = 0x1e0;
    ClearImage(&local_50,'\0','\0','\0');
    local_44 = 0;
    iRamffffacb0 = 1;
    iRam00000624 = 0;
LAB_80014224:
    do {
      uVar8 = 0;
      local_30 = 0;
      local_44 = local_44 + 1;
      if ((((local_44 & 7) == 0) && (iVar7 = FUN_80043bb4(), iVar7 != 0)) &&
         (iVar7 = FUN_80012a90(iRam00000628,0), iVar7 != 0)) goto LAB_80014ebc;
      uVar15 = 2;
      if (iRam00000618 == 0) {
        if (iRam0000001c == 0) {
          uVar15 = 1;
        }
        else {
          uVar15 = iRamffffacb0 - uRam0000000c;
        }
      }
      uVar16 = 0;
      if (uVar15 != 0) {
        do {
          FUN_800120d4();
          uRam0000000c = uRam0000000c + 1;
          uRam000006cc = (undefined2)uRam0000000c;
          uVar9 = 0;
          if (uVar16 == uVar15 - 1) {
            uVar9 = uVar15;
          }
          FUN_8002131c(uVar9);
          FUN_80021394(uRam0000000c);
          FUN_80021678();
          if ((DAT_80065c30 & 0x800000) != 0) {
            iVar4 = 3 - iVar4;
          }
          uVar16 = uVar16 + 1;
          uVar8 = uVar8 | uRam0000062c;
          local_30 = local_30 | uRam00000630;
        } while (uVar16 < uVar15);
      }
      FUN_800212c4(uRam0000000c & 0xffff);
      iRam00000008 = 1 - iRam00000008;
      if ((short)puRam000007d0[3] == 0) {
        iVar4 = 0;
      }
      else if ((iVar4 == 2) && ((*puRam000007d0 & 0x1000000) != 0)) {
        iVar4 = 1;
      }
      if (puRam000007d4 != (uint *)0x0) {
        if ((short)puRam000007d4[3] == 0) {
          iVar18 = 0;
        }
        else if ((iVar18 == 2) && ((*puRam000007d4 & 0x1000000) != 0)) {
          iVar18 = 1;
        }
      }
      if (iRam00000010 == 0) {
        if (((char)bRam00000015 < '\x03') || ((short)puRam000007d0[3] != 0)) {
          puVar17 = puRam000007d0;
          iVar7 = iVar4;
          if (puRam000007d4 != (uint *)0x0) {
            uVar16 = *puRam000007d4;
            puVar11 = puRam000007d4;
            goto LAB_80014a54;
          }
        }
        else {
          uVar16 = *puRam000007d0;
          puVar11 = puRam000007d0;
          puVar17 = puRam000007d4;
          iVar7 = iVar18;
LAB_80014a54:
          if ((uVar16 & 0x1000000) == 0) {
            *puVar11 = uVar16 & 0xfffffffd;
          }
        }
        if (iVar7 == 2) {
          uVar16 = puVar17[0x3e];
          *puVar17 = *puVar17 | 2;
          sVar1 = *(short *)(puVar17[0x38] + 0x8a);
        }
        else {
          if ((*puVar17 & 0x1000000) == 0) {
            *puVar17 = *puVar17 & 0xfffffffd;
          }
          uVar16 = puVar17[0x38];
          sVar1 = *(short *)(uVar16 + 0x8a);
        }
        FUN_8001db24(uVar16,(int)sVar1);
        FUN_800119c0(iRam00000008);
        FUN_8001d994(0x140,0xf0,0xa0,0x78);
        FUN_80021600();
        if ((iVar7 == 2) && ((*puVar17 & 0x20000000) == 0)) {
          FUN_8002b8d0(puVar17);
        }
        ClearOTagR(&local_48,1);
        uVar3 = local_48;
        if (uRam00000680 != 0) {
          local_48 = 0x6eed8;
          DAT_8006eee4 = DAT_8006eee4 & 0xff000000 | uRam00000680;
          DAT_8006eed8 = DAT_8006eed8 & 0xff000000 | uVar3;
        }
        uRam00000680 = 0;
        if (iVar7 != 0) {
          FUN_8002af98(puVar17,2 - iVar7,&local_48);
        }
        FUN_8002b7bc(puVar17,&DAT_8006f680,&local_48);
        if (iRam00000618 == 0) {
          FUN_80019d10(&DAT_8006eef0,iRam00000628,&local_48,uVar15);
        }
        else if ((uRam0000000c & 0x3f) < 0x28) {
          *(undefined1 *)(iRam00000628 + 4) = 0x80;
          *(undefined1 *)(iRam00000628 + 5) = 0x80;
          *(undefined1 *)(iRam00000628 + 6) = 0;
          FUN_80019c64(iRam00000628,"DEMO MODE",&DAT_80065618,10);
        }
        if (iVar19 == 0) {
          if ((iRam00000624 != 0) && (DAT_8006eef0 == 0)) {
            if (iRam000005ac == 4) {
              if (iRam00000024 == 0) {
                pcVar5 = "Sounds\\Defeat.xa";
              }
              else {
                pcVar5 = "Sounds\\Victory.xa";
              }
              FUN_80043df8(pcVar5,(int)DAT_80065674);
            }
            if (bRam00000015 == 0) {
              uRam00000620 = FUN_800220d4();
              uRam00000620 = uRam00000620 ^ DAT_80065674 < '\x06';
            }
            iVar19 = FUN_8001392c(iRam00000628);
          }
        }
        else {
          FUN_80018f7c(iVar19,&local_48);
        }
        FUN_800128bc();
        DrawSync(0);
        FUN_8002a25c(0,(int)*(short *)(&DAT_8006f20a + (1 - iRam00000004) * 0x5c),&local_48);
        if (DAT_80065c28 < 2) {
          uVar8 = uVar8 | 0x8000000;
        }
        FUN_80012828(&DAT_8006f5a0 + iRam00000008 * 0x14,&DAT_8006f208 + iRam00000004 * 0x5c,
                     &local_48,iRam0000060c + 0x3ffc);
      }
      else {
        uVar6 = 0xf0;
        if (iRam00000010 == 1) {
          uVar10 = 0x140;
          uVar6 = 0x78;
          uVar13 = 0xa0;
          uVar14 = 0x3c;
        }
        else {
          uVar10 = 0xa0;
          uVar13 = 0x50;
          uVar14 = 0x78;
        }
        FUN_8001d994(uVar10,uVar6,uVar13,uVar14);
        FUN_800119c0(0);
        if ((*puRam000007d4 & 0x1000000) == 0) {
          *puRam000007d4 = *puRam000007d4 & 0xfffffffd;
        }
        if (iVar4 == 2) {
          puVar17 = puRam000007d0 + 0x3e;
          *puRam000007d0 = *puRam000007d0 | 2;
          sVar1 = *(short *)(local_40 + 0x8a);
          uVar16 = *puVar17;
        }
        else {
          sVar1 = *(short *)(local_40 + 0x8a);
          uVar16 = local_40;
        }
        FUN_8001db24(uVar16,(int)sVar1);
        DAT_8006f6a0 = DAT_8006f680;
        DAT_8006f6a4 = DAT_8006f684;
        DAT_8006f6a8 = DAT_8006f688;
        DAT_8006f6ac = DAT_8006f68c;
        DAT_8006f6b0 = DAT_8006f690;
        DAT_8006f6b4 = DAT_8006f694;
        DAT_8006f6b8 = DAT_8006f698;
        DAT_8006f6bc = DAT_8006f69c;
        FUN_80021600();
        ClearOTagR(&local_48,1);
        iVar7 = FUN_80019d10(&DAT_8006eef0,iRam00000628,&local_48,uVar15);
        if ((iVar7 == 0) && (iVar19 == 0)) {
          if ((((iRam00000624 != 0) && (DAT_8006eef0 == 0)) && (DAT_8006eff8 == 0)) &&
             (DAT_8006f100 == 0)) {
            if (iRam000005ac == 4) {
              if ((iRam00000024 == 0) && (bRam00000015 != 3)) {
                pcVar5 = "Sounds\\Defeat.xa";
              }
              else {
                pcVar5 = "Sounds\\Victory.xa";
              }
              uVar16 = 0;
              if (bRam00000015 == 3) {
                uVar16 = (uint)((short)puRam000007d0[3] == 0);
              }
              FUN_80043df8(pcVar5,(int)(&DAT_80065674)[uVar16]);
            }
            iVar19 = FUN_8001392c(iRam00000628);
          }
        }
        else {
          if (iVar19 != 0) {
            FUN_80018f7c(iVar19,&local_48);
          }
          uVar3 = local_48;
          local_48 = (uint)(&uStackY_f0 + iRam00000010 * 8 + iRam00000008 * 4) & 0xffffff;
          (&uStackY_f0)[iRam00000010 * 8 + iRam00000008 * 4] =
               (uint)*(byte *)((int)&uStackY_f0 + iRam00000008 * 0x10 + iRam00000010 * 0x20 + 3) <<
               0x18 | uVar3;
          iVar7 = (1 - iRam00000008) * 0x5c;
          SetDrawEnv((DR_ENV *)(&DAT_8006f224 + iVar7),(DRAWENV *)(&DAT_8006f208 + iVar7));
          iVar7 = (1 - iRam00000008) * 0x5c;
          *(uint *)(&DAT_8006f224 + iVar7) = (uint)(byte)(&DAT_8006f227)[iVar7] << 0x18 | local_48;
          local_48 = (uint)(&DAT_8006f224 + iVar7) & 0xffffff;
        }
        if (iVar18 != 0) {
          FUN_8002af98(puRam000007d4,iRam00000010 << 1 | 1,&local_48);
        }
        FUN_8002b7bc(puRam000007d4,&DAT_8006f6c0,&local_48);
        FUN_80019d10(&DAT_8006f100,iRam00000628,&local_48,uVar15);
        DrawSync(0);
        iVar7 = (iRam00000010 * 4 - (iRam00000008 + -1)) * 0x5c;
        FUN_8002a25c((int)*(short *)(&DAT_8006f208 + iVar7),(int)*(short *)(&DAT_8006f20a + iVar7),
                     &local_48);
        FUN_80012828(&DAT_8006f5a0 + iRam00000008 * 0x14,
                     &UNK_8006f150 + (iRam00000010 * 4 + iRam00000008) * 0x5c,&local_48,
                     iRam0000060c + 0x3ffc);
        FUN_800119c0(1);
        if ((*puRam000007d0 & 0x1000000) == 0) {
          *puRam000007d0 = *puRam000007d0 & 0xfffffffd;
        }
        if (iVar18 == 2) {
          puVar17 = puRam000007d4 + 0x3e;
          *puRam000007d4 = *puRam000007d4 | 2;
          sVar1 = *(short *)(local_3c + 0x8a);
          uVar16 = *puVar17;
        }
        else {
          sVar1 = *(short *)(local_3c + 0x8a);
          uVar16 = local_3c;
        }
        FUN_8001db24(uVar16,(int)sVar1);
        DAT_8006f6c0 = DAT_8006f680;
        DAT_8006f6c4 = DAT_8006f684;
        DAT_8006f6c8 = DAT_8006f688;
        DAT_8006f6cc = DAT_8006f68c;
        DAT_8006f6d0 = DAT_8006f690;
        DAT_8006f6d4 = DAT_8006f694;
        DAT_8006f6d8 = DAT_8006f698;
        DAT_8006f6dc = DAT_8006f69c;
        FUN_80021600();
        ClearOTagR(&local_48,1);
        if (iVar4 != 0) {
          FUN_8002af98(puRam000007d0,iRam00000010 << 1,&local_48);
        }
        FUN_8002b7bc(puRam000007d0,&DAT_8006f6a0,&local_48);
        FUN_80019d10(&DAT_8006eff8,iRam00000628,&local_48,uVar15);
        FUN_800128bc();
        DrawSync(0);
        iVar7 = (iRam00000010 * 4 + iRam00000008 + -2) * 0x5c;
        FUN_8002a25c((int)*(short *)(&DAT_8006f208 + iVar7),(int)*(short *)(&DAT_8006f20a + iVar7),
                     &local_48);
        DrawOTag(&local_48);
        DrawSync(0);
        if (DAT_80065c28 < 2) {
          uVar8 = uVar8 | 0x8000000;
        }
        if (DAT_80065c40 < 2) {
          local_30 = local_30 | 0x8000000;
        }
        PutDrawEnv((DRAWENV *)(&DAT_8006f208 + (iRam00000010 * 4 + iRam00000008) * 0x5c));
        DrawOTag((u_long *)(iRam0000060c + 0x3ffc));
      }
      if ((iVar19 != 0) && (iRam00000624 = iRam00000624 + uVar15, 300 < iRam00000624)) {
        if (bRam00000015 == 0) {
          if (((uVar8 & 0x8400000) != 0) || ((iRam00000024 == 0 && (0x4b0 < iRam00000624))))
          goto LAB_80014ebc;
        }
        else if ((((uVar8 | local_30) & 0x8600000) != 0) || (0x4b0 < iRam00000624)) {
          local_38 = (uVar8 | local_30) & 0x200000;
          goto LAB_80014ebc;
        }
      }
      uVar16 = uVar8 | local_30;
      if ((uVar16 & 0x100) == 0) {
        if (((uVar16 & 0x8000000) != 0) &&
           ((iRam00000618 != 0 ||
            (iVar7 = FUN_80012a90(iRam00000628,(uVar8 >> 0x1b ^ 1) & 1), iVar7 != 0)))) break;
        goto LAB_80014224;
      }
      if ((uVar16 & 0x800) == 0) {
        local_34 = 0x78;
        goto LAB_80014224;
      }
      local_34 = local_34 - uVar15;
    } while (-1 < local_34);
    uRam00000014 = 3;
LAB_80014ebc:
    if (iVar19 != 0) {
      FUN_80018f3c(iVar19);
    }
    if (iRam00000618 != 0) {
      FUN_8001265c();
      FUN_800120d4();
      uRam00000014 = 3;
      if ((uRam0000062c == 0) && (uRam00000014 = 3, uRam00000630 == 0)) {
        uRam00000014 = 2;
      }
      FUN_80011c58(&DAT_80065968);
      FUN_800126c8();
    }
    FUN_800128bc();
    FUN_80044054();
    FUN_80044394(uRam000005f8);
    FUN_80022a1c();
    FUN_800204dc(local_40);
    if (puRam000007d4 != (uint *)0x0) {
      FUN_800204dc(local_3c);
    }
    FUN_8002accc();
    FUN_80041e80();
    FUN_8001356c(iRam00000628);
    FUN_800190d8(iRam00000628);
    FUN_80011914(0);
    FUN_80011914(1);
    FUN_80016678(0);
  } while( true );
}

