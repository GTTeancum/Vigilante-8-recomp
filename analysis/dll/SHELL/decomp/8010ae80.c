// addr: 0x8010ae80  name: FUN_8010ae80

/* WARNING: Removing unreachable block (ram,0x8010bdfc) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010ae80(int param_1)

{
  short sVar1;
  bool bVar2;
  ushort uVar3;
  undefined4 uVar4;
  byte *pbVar5;
  int iVar6;
  undefined1 *puVar7;
  uint uVar8;
  int iVar9;
  char *pcVar10;
  uint uVar11;
  uint extraout_v1;
  uint extraout_v1_00;
  undefined1 *puVar12;
  uint uVar13;
  undefined4 uVar14;
  uint uVar15;
  char *pcVar16;
  ushort *puVar17;
  int iVar18;
  ushort *puVar19;
  undefined1 uVar20;
  uint uVar22;
  int iVar23;
  uint uVar24;
  uint uVar25;
  uint uVar26;
  undefined8 uVar27;
  int local_res0;
  undefined1 auStack_170 [16];
  undefined4 local_160;
  undefined4 local_15c;
  undefined2 local_158;
  undefined2 local_156;
  undefined2 local_154;
  undefined2 local_152;
  undefined1 auStack_150 [128];
  undefined1 auStack_d0 [8];
  undefined1 auStack_c8 [8];
  char local_c0 [16];
  undefined4 local_b0;
  undefined1 auStack_a8 [8];
  undefined4 local_a0;
  undefined4 local_98;
  byte local_90 [64];
  uint local_50;
  int local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  uint local_3c;
  int local_38;
  undefined1 *local_34;
  int *local_30;
  int iVar21;
  
  local_48 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100d7c,0xb8,0x80,0);
  uVar24 = 0;
  local_3c = 0;
  local_44 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,6,0x80);
  local_40 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,0x25,0x80,8);
  uVar4 = DAT_80100cbc;
  uVar14 = DAT_80100cb8;
  local_38 = *(int *)(local_44 + 0x50);
  *(undefined4 *)(local_44 + 0x48) = DAT_80100cb4;
  *(undefined4 *)(local_44 + 0x4c) = uVar14;
  *(undefined4 *)(local_44 + 0x50) = uVar4;
  FUN_8001dc1c/*0x8001dc1c*/();
  *(undefined2 *)(local_44 + 0x40) = 0xfeab;
  uVar25 = 0;
  uVar26 = 0;
  iVar23 = 2;
  local_34 = auStack_150;
  *(undefined4 *)(local_40 + 0x2c) = 0;
  *(undefined4 *)(local_40 + 0x28) = 0;
  *(undefined4 *)(local_40 + 0x24) = 0;
  *(undefined2 *)(*(int *)(local_40 + 0x30) + 0x28) = 0xffc0;
  local_res0 = param_1;
  do {
    FUN_80019e20/*0x80019e20*/();
    if (iVar23 == 2) {
      iVar23 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
      uVar14 = 0x31;
      local_158 = 0xe0;
      local_156 = 0x30;
      local_154 = 0x1a0;
      local_152 = 0x38;
      if (uVar26 != 0) {
        uVar14 = 0x32;
      }
      sprintf/*0x80053004*/(local_34,s_MEMORY_CARD_SLOT__c_80100dc0,uVar14);
      FUN_80019e20/*0x80019e20*/();
      FUN_8001a0ac/*0x8001a0ac*/(&local_158,0);
      *(undefined1 *)(iVar23 + 4) = 0x32;
      *(undefined1 *)(iVar23 + 5) = 0x73;
      *(undefined1 *)(iVar23 + 6) = 100;
      FUN_80019a58/*0x80019a58*/(iVar23,local_34,&local_158,2);
      FUN_800190a8/*0x800190a8*/(iVar23);
      iVar23 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
      *(undefined1 *)(iVar23 + 6) = 100;
      *(undefined1 *)(iVar23 + 4) = 0x32;
      *(undefined1 *)(iVar23 + 5) = 0x73;
      FUN_80019a58/*0x80019a58*/(iVar23,(&PTR_s__CHECKING__80112298)[(byte)(&DAT_801133b0)[(int)uVar26 >> 4]],
                      &local_158,6);
      FUN_800190a8/*0x800190a8*/(iVar23);
      uVar14 = 0x31;
      if (uVar26 != 0) {
        uVar14 = 0x32;
      }
      sprintf/*0x80053004*/(local_34,(&PTR_s__CHECKING__80112298)[(byte)(&DAT_801133b0)[(int)uVar26 >> 4]]
                      ,uVar14);
      pbVar5 = &DAT_80112274;
      uVar22 = 0;
      do {
        puVar17 = (ushort *)(DAT_801133a4 + (uVar26 + uVar22) * 0x200);
        func_0x800188ec(DAT_80113394 + (uint)*pbVar5 * 3 + 3,auStack_d0,auStack_c8);
        uVar8 = (uint)*puVar17;
        uVar27 = CONCAT44(uVar8,0x5858);
        if (uVar8 == 0x4353) {
          uVar27 = LoadImage/*0x8004f82c*/(auStack_d0,puVar17 + 0x40);
        }
        if ((int)((ulonglong)uVar27 >> 0x20) == (int)uVar27) {
          FUN_800185cc/*0x800185cc*/(local_c0,*(undefined4 *)(*(int *)(*DAT_80113394 + 0x14) + 0xb0));
          LoadImage/*0x8004f82c*/(auStack_d0,local_b0);
        }
        FUN_800185cc/*0x800185cc*/(auStack_a8,*(undefined4 *)(*(int *)(*DAT_80113394 + 0x14) + 0x50));
        LoadImage/*0x8004f82c*/(auStack_d0,local_98);
        uVar8 = uVar22 + 1;
        LoadImage/*0x8004f82c*/(auStack_c8,local_a0);
        pbVar5 = &DAT_80112275 + uVar22;
        uVar22 = uVar8;
      } while (uVar8 < 0xf);
      iVar23 = 0;
    }
    iVar6 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_80112284,0);
    iVar21 = 0x80;
    uVar20 = 0x80;
    *(undefined1 *)(iVar6 + 4) = 0x80;
    *(undefined1 *)(iVar6 + 5) = 0x80;
    *(undefined1 *)(iVar6 + 6) = 0x80;
    if (local_res0 != 0) {
      if (local_3c != 0) {
        local_c0[0] = s_empty_80100dd4[0];
        local_c0[1] = s_empty_80100dd4[1];
        local_c0[2] = s_empty_80100dd4[2];
        local_c0[3] = s_empty_80100dd4[3];
        local_c0[5] = s_empty_80100dd4[5];
        local_c0[4] = s_empty_80100dd4[4];
        V8_MemSet/*0x80044efc*/(local_c0 + 6,0,0x1b);
        FUN_8001d564/*0x8001d564*/(local_40);
        iVar18 = *(int *)(local_44 + 0x38);
        if (iVar18 != 0) {
          iVar21 = uVar24 + 1;
          do {
            iVar9 = local_38;
            if (*(short *)(iVar18 + 6) == iVar21) {
              iVar9 = local_38 + 0xc00;
            }
            *(int *)(iVar18 + 0x2c) = iVar9;
            if (*(short *)(iVar18 + 6) == iVar21) {
              FUN_8001d4f0/*0x8001d4f0*/(iVar18,local_40);
            }
            iVar18 = *(int *)(iVar18 + 0x34);
          } while (iVar18 != 0);
        }
        uVar20 = (undefined1)iVar21;
        puVar17 = (ushort *)((uVar26 + uVar24) * 0x200 + DAT_801133a4);
        puVar12 = (undefined1 *)(uint)*puVar17;
        puVar7 = (undefined1 *)0x5858;
        if (puVar12 == (undefined1 *)0x4353) {
          puVar17 = puVar17 + 2;
          uVar22 = 0;
          do {
            uVar3 = *puVar17;
            puVar19 = puVar17;
            if ((char)*puVar17 < '\0') {
              puVar19 = puVar17 + 1;
              local_c0[uVar22] = *(char *)((int)puVar17 + 1);
            }
            puVar17 = (ushort *)((int)puVar19 + 1);
            local_c0[uVar22] = (char)uVar3;
          } while (((char)uVar3 != '\0') && (uVar22 = uVar22 + 1, uVar22 < 0x20));
          uVar20 = (undefined1)uVar22;
          puVar7 = auStack_170 + uVar22;
          puVar12 = (undefined1 *)0x1;
          local_c0[uVar22] = '\0';
        }
        if (puVar12 == puVar7) {
          local_c0[0] = s_used_block_80100ddc[0];
          local_c0[1] = s_used_block_80100ddc[1];
          local_c0[2] = s_used_block_80100ddc[2];
          local_c0[3] = s_used_block_80100ddc[3];
          local_c0[4] = s_used_block_80100ddc[4];
          local_c0[5] = s_used_block_80100ddc[5];
          local_c0[6] = s_used_block_80100ddc[6];
          local_c0[7] = s_used_block_80100ddc[7];
          local_c0[8] = s_used_block_80100ddc[8];
          local_c0[9] = s_used_block_80100ddc[9];
          local_c0[10] = s_used_block_80100ddc[10];
        }
      }
      FUN_80019a58/*0x80019a58*/(iVar6,(&PTR_DAT_8011228c)[uVar24],&DAT_801122ac,2);
      *(undefined1 *)(iVar6 + 4) = 0;
      *(undefined1 *)(iVar6 + 5) = uVar20;
      *(undefined1 *)(iVar6 + 6) = 0;
      FUN_80019a58/*0x80019a58*/(iVar6,&DAT_801007a8,&DAT_801122ac,0);
      FUN_80019a58/*0x80019a58*/(iVar6,&DAT_801007ac,&DAT_801122ac,1);
    }
    local_30 = &local_4c;
    do {
      iVar18 = (int)_DAT_800659d0;
      _DAT_800659d0 = _DAT_800659d0 + 1;
      iVar21 = (0x4000000 - *(int *)(local_44 + 0x48)) + iVar23 * -0x40000;
      FUN_8001fcb4/*0x8001fcb4*/(local_40,iVar18);
      iVar18 = iVar21;
      if (iVar21 < 0) {
        iVar18 = iVar21 + 0x1f;
      }
      uVar22 = (uint)*(ushort *)(local_44 + 0x42);
      *(int *)(local_44 + 0x48) = *(int *)(local_44 + 0x48) + (iVar18 >> 5);
      sVar1 = *(short *)(local_44 + 0x42);
      if ((local_res0 != 0) && (local_3c != 0)) {
        iVar18 = (int)(uVar22 * -0x100000) >> 0x10;
        if (iVar18 < 0) {
          iVar18 = iVar18 + 0xff;
        }
        uVar22 = 1;
        *(short *)(local_44 + 0x42) = sVar1 + (short)((uint)iVar18 >> 8);
      }
      *(short *)(local_44 + 0x42) = sVar1 + 0x11;
      if (iVar23 != 0) {
        if (iVar21 < 0) {
          iVar21 = -iVar21;
        }
        if (iVar21 < 0x20000) {
          uVar24 = uVar24 + iVar23 * -2;
          iVar21 = iVar23 * 0x20000;
          iVar23 = 2;
          *(int *)(local_44 + 0x48) = iVar21 + 0x4000000;
          uVar26 = 0x10 - uVar26;
        }
      }
      Buffer_StartOTagOther/*0x80011a10*/(uVar22);
      FUN_8001d708/*0x8001d708*/(local_44);
      FUN_8001a4f8/*0x8001a4f8*/(local_48,0);
      FUN_8001de08/*0x8001de08*/(local_44);
      if ((local_res0 != 0) && (local_3c != 0)) {
        *(undefined1 *)(iVar6 + 4) = 0;
        *(undefined1 *)(iVar6 + 5) = 0x80;
        *(undefined1 *)(iVar6 + 6) = 0;
        FUN_80019960/*0x80019960*/(iVar6,&DAT_801007a8,0x20);
        FUN_80019960/*0x80019960*/(iVar6,&DAT_801007ac,0x140,0x80);
      }
      iVar21 = DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      if ((iVar21 == -1) || (iVar21 != 1)) {
        uVar25 = uVar26;
      }
      iVar21 = (int)uVar25 >> 4;
      if (local_4c == 1) {
LAB_8010b620:
        if ((&DAT_801133b0)[iVar21] != '\x01') {
          (&DAT_801133b0)[iVar21] = '\x01';
          V8_MemSet/*0x80044efc*/(DAT_801133a4 + uVar25 * 0x200,0,0x2000);
          V8_MemSet/*0x80044efc*/(DAT_801133a0 + uVar25 * 0x28,0,0x280);
LAB_8010b684:
          pcVar16 = &DAT_801133b0;
          pcVar10 = &DAT_801133b0 + iVar21;
          if (*pcVar10 != '\0') {
            *pcVar10 = '\0';
            V8_MemSet/*0x80044efc*/(DAT_801133a4 + uVar25 * 0x200,0,0x2000);
            uVar27 = V8_MemSet/*0x80044efc*/(DAT_801133a0 + uVar25 * 0x28,0,0x280);
            pcVar10 = (char *)((ulonglong)uVar27 >> 0x20);
            iVar18 = (int)uVar27;
            if (iVar23 == 0) {
              iVar23 = 2;
              goto LAB_8010b6f4;
            }
          }
LAB_8010b7b8:
          iVar21 = DAT_801133a4;
          if (pcVar10 == pcVar16) {
            if (((DAT_801133a8 & 0xf) < 0xf) &&
               (*(char *)((DAT_801133a8 * 5 << ((uint)pcVar16 & 0x1f)) + DAT_801133a0) != '\0')) {
              uVar25 = DAT_801133a8 & 0x10;
            }
            uVar22 = DAT_801133a8 & 0x10;
            pcVar10 = (char *)(uVar22 * 0x28 + DAT_801133a0);
            do {
              if (*pcVar10 == '\0') break;
              iVar23 = *(int *)(pcVar10 + 0x18) + 0x1fff;
              if (iVar23 < 0) {
                iVar23 = *(int *)(pcVar10 + 0x18) + 0x3ffe;
              }
              for (iVar23 = (iVar23 >> 0xd) + -2; iVar23 != -1; iVar23 = iVar23 + -1) {
                iVar18 = DAT_801133a8 * 0x200;
                DAT_801133a8 = DAT_801133a8 + 1;
                *(undefined2 *)(iVar18 + iVar21) = 0x5858;
              }
              uVar22 = uVar22 + 1;
              pcVar10 = pcVar10 + 0x28;
            } while ((uVar22 & 0xf) < 0xf);
            (&DAT_801133b0)[(int)uVar25 >> 4] = 4;
            goto LAB_8010b8e4;
          }
        }
      }
      else {
        if (local_4c != 0) {
          if (local_4c == 3) goto LAB_8010b684;
          iVar18 = iVar21;
          if (local_4c != 4) goto LAB_8010b620;
LAB_8010b6f4:
          (&DAT_801133b0)[iVar18] = 2;
          V8_MemSet/*0x80044efc*/(DAT_801133a4 + uVar25 * 0x200,0,0x2000);
          V8_MemSet/*0x80044efc*/(DAT_801133a0 + uVar25 * 0x28,0,0x280);
        }
        if (local_50 == 2) {
LAB_8010b8e4:
          (&DAT_801133b0)[(int)uVar25 >> 4] = 3;
          iVar23 = 2;
          local_160 = 0;
          local_15c = 0xf;
          DAT_801133a8 = uVar25;
        }
        else {
          iVar18 = 0;
          if (((local_50 < 3) && (iVar18 = iVar21, local_50 == 1)) ||
             (iVar21 = iVar18, local_50 != 3)) {
            pcVar10 = (char *)(uint)(byte)(&DAT_801133b0)[iVar21];
            pcVar16 = (char *)0x3;
            goto LAB_8010b7b8;
          }
        }
        DAT_801133a8 = DAT_801133a8 + 1;
        if (iVar23 == 0) {
          iVar23 = 2;
        }
      }
      VSync/*0x80047e44*/(0);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(local_48);
      Pad_Tick/*0x800120d4*/();
      uVar22 = _DAT_80065930 | _DAT_80065934;
    } while ((iVar23 != 2) && ((uVar22 & 0xf8d00000) == 0));
    FUN_800190a8/*0x800190a8*/(iVar6);
    if ((iVar23 == 2) ||
       (((local_res0 == 0 || (local_3c == 0)) || (uVar14 = 0xc, (uVar22 & 0xf0000000) == 0)))) {
      uVar14 = 0xe;
    }
    FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,uVar14);
    uVar8 = 0x50100000;
    if (local_res0 == 0) {
LAB_8010bf00:
      if ((uVar22 & uVar8) != 0) break;
      if ((uVar22 & 0x8400000) != 0) {
        uVar24 = 0;
        local_res0 = 1;
      }
    }
    else {
      if (local_3c == 0) {
LAB_8010be80:
        if ((int)uVar22 < 0) {
          uVar24 = uVar24 - 1;
        }
        uVar8 = uVar24 + 3;
        if ((uVar22 & 0x20000000) != 0) {
          uVar8 = uVar24 + 4;
        }
        uVar24 = uVar8 % 3;
        if ((uVar22 & 0x8400000) != 0) {
          local_3c = uVar24 + 1;
          uVar24 = 0;
        }
      }
      else {
        if (((uVar22 & 0x10000000) != 0) && (2 < uVar24)) {
          uVar24 = uVar24 - 3;
        }
        if (((uVar22 & 0x40000000) != 0) && (uVar24 < 0xc)) {
          uVar24 = uVar24 + 3;
        }
        if ((int)uVar22 < 0) {
          if (uVar24 != (uVar24 / 3) * 3) {
            uVar24 = uVar24 - 1;
          }
          iVar23 = -1;
        }
        if ((uVar22 & 0x20000000) != 0) {
          if (uVar24 % 3 < 2) {
            uVar24 = uVar24 + 1;
          }
          iVar23 = 1;
        }
        if ((uVar22 & 0x8400000) != 0) {
          pbVar5 = &DAT_801133b0 + ((int)uVar26 >> 4);
          uVar8 = (uint)*pbVar5;
          uVar13 = 3;
          if ((uVar8 == 3) || (uVar11 = 1, uVar8 == 0)) {
            FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0xb);
            uVar13 = 1;
            uVar8 = 0x100000;
            uVar11 = extraout_v1;
          }
          uVar15 = 2;
          if (uVar8 == uVar11) {
            pbVar5 = local_90;
            if (uVar26 != 0) {
              uVar11 = 2;
            }
            sprintf/*0x80053004*/(pbVar5,s_There_is_no_Memory_card_in_Memor_80100de8,uVar11);
            uVar8 = 0x100000;
            if (local_3c != 2) goto LAB_8010bee0;
            uVar13 = 1;
            uVar11 = extraout_v1_00;
          }
          if (local_3c == uVar15) {
LAB_8010bc50:
            if ((uVar8 == local_3c) && (uVar8 == 0)) {
              *pbVar5 = 4;
            }
            if (*(short *)((uVar26 + uVar24) * 0x200 + DAT_801133a4) != 0) {
              iVar23 = thunk_FUN_800523a0/*0x80052384*/(DAT_801133a0 + (uVar26 + uVar24) * 0x28,
                                       s_BASLUS_00510_80100d1c,0xc);
              uVar13 = 0;
              bVar2 = false;
              uVar8 = uVar26;
              if (iVar23 == 0) goto LAB_8010bd50;
              do {
                if (*(short *)(uVar8 * 0x200 + DAT_801133a4) == 0) break;
                uVar13 = uVar13 + 1;
                uVar8 = uVar26 + uVar13;
              } while (uVar13 < 0xf);
              do {
                bVar2 = true;
LAB_8010bd50:
              } while (bVar2);
            }
            iVar23 = 2;
            uVar14 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 0xc),1);
            FUN_800190a8/*0x800190a8*/(uVar14,uVar14,0x1c0,0xf0);
          }
          else {
            if (local_3c < 3) {
              if (local_3c != uVar11) {
                uVar13 = 1;
                goto LAB_8010bbc4;
              }
LAB_8010bbdc:
              DAT_801133b4 = thunk_FUN_800523a0/*0x80052384*/(DAT_801133a0 + (uVar26 + uVar24) * 0x28,
                                             s_BASLUS_00510_80100d1c,0xc);
              uVar8 = 0x100000;
              goto LAB_8010bc50;
            }
LAB_8010bbc4:
            if (local_3c != uVar13) goto LAB_8010bbdc;
          }
          if (*(short *)((uVar26 + uVar24) * 0x200 + DAT_801133a4) != 0x4353) goto LAB_8010be80;
        }
      }
LAB_8010bee0:
      if ((uVar22 & 0x100000) != 0) {
        local_3c = 0;
        local_res0 = 0;
        uVar8 = 0x800000;
        goto LAB_8010bf00;
      }
    }
  } while ((uVar22 & 0x800000) == 0);
  func_0x8001a4ac(local_48);
  FUN_8001d564/*0x8001d564*/(local_40);
  FUN_8001af48/*0x8001af48*/(local_44);
  FUN_8001af48/*0x8001af48*/(local_40);
  FUN_80019e20/*0x80019e20*/();
  return;
}

