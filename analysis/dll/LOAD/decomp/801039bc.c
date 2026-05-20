// addr: 0x801039bc  name: FUN_801039bc

void FUN_801039bc(uint *param_1,int param_2,int param_3)

{
  longlong lVar1;
  uint uVar2;
  int iVar3;
  uint *puVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  uint uVar13;
  uint local_188 [27];
  uint local_11c;
  uint local_118;
  undefined4 local_114;
  uint local_110;
  longlong local_30;
  
  uVar12 = 0x10000;
  iVar11 = 0;
  uVar8 = 0x8000;
  do {
    uVar2 = param_1[3];
    uVar9 = 0x10000 - uVar8;
    iVar10 = -(uint)(0x10000 < uVar8) - iVar11;
    uVar7 = param_1[6];
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar9 * (ulonglong)uVar2) + uVar13;
    local_188[0x1a] =
         __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar9 * (ulonglong)uVar2 >> 0x20) +
                               uVar9 * ((int)uVar2 >> 0x1f) + uVar2 * iVar10 +
                               (int)((ulonglong)uVar8 * (ulonglong)uVar7 >> 0x20) +
                               uVar8 * ((int)uVar7 >> 0x1f) + uVar7 * iVar11 +
                               (uint)(uVar5 < uVar13),0x10000,0);
    uVar2 = param_1[5];
    uVar7 = param_1[8];
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar9 * (ulonglong)uVar2) + uVar13;
    local_188[0x19] =
         __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar9 * (ulonglong)uVar2 >> 0x20) +
                               uVar9 * ((int)uVar2 >> 0x1f) + uVar2 * iVar10 +
                               (int)((ulonglong)uVar8 * (ulonglong)uVar7 >> 0x20) +
                               uVar8 * ((int)uVar7 >> 0x1f) + uVar7 * iVar11 +
                               (uint)(uVar5 < uVar13),0x10000,0);
    local_188[0x18] = local_188[0x1a];
    local_188[0] = *param_1;
    local_188[1] = param_1[1];
    local_188[2] = param_1[2];
    local_188[0x15] = param_1[9];
    local_188[0x16] = param_1[10];
    local_188[0x17] = param_1[0xb];
    uVar2 = *param_1;
    uVar7 = param_1[3];
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar9 * (ulonglong)uVar2) + uVar13;
    local_11c = local_188[0x19];
    local_118 = __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar9 * (ulonglong)uVar2 >> 0x20) +
                                      uVar9 * ((int)uVar2 >> 0x1f) + uVar2 * iVar10 +
                                      (int)((ulonglong)uVar8 * (ulonglong)uVar7 >> 0x20) +
                                      uVar8 * ((int)uVar7 >> 0x1f) + uVar7 * iVar11 +
                                      (uint)(uVar5 < uVar13),0x10000,0);
    local_114 = 0;
    uVar2 = param_1[2];
    uVar7 = param_1[5];
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar9 * (ulonglong)uVar2) + uVar13;
    local_188[5] = __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar9 * (ulonglong)uVar2 >> 0x20) +
                                         uVar9 * ((int)uVar2 >> 0x1f) + uVar2 * iVar10 +
                                         (int)((ulonglong)uVar8 * (ulonglong)uVar7 >> 0x20) +
                                         uVar8 * ((int)uVar7 >> 0x1f) + uVar7 * iVar11 +
                                         (uint)(uVar5 < uVar13),0x10000,0);
    local_188[3] = local_118;
    local_188[4] = local_114;
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)local_188[0x18]);
    uVar2 = (int)((ulonglong)uVar9 * (ulonglong)local_118) + uVar13;
    local_110 = local_188[5];
    local_188[0x1a] =
         __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar9 * (ulonglong)local_118 >> 0x20) +
                               uVar9 * ((int)local_118 >> 0x1f) + local_118 * iVar10 +
                               (int)((ulonglong)uVar8 * (ulonglong)local_188[0x18] >> 0x20) +
                               uVar8 * ((int)local_188[0x18] >> 0x1f) + local_188[0x18] * iVar11 +
                               (uint)(uVar2 < uVar13),0x10000,0);
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)local_188[0x19]);
    local_11c = 0;
    uVar2 = (int)((ulonglong)uVar9 * (ulonglong)local_188[5]) + uVar13;
    local_188[8] = __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar9 * (ulonglong)local_188[5] >> 0x20) +
                                         uVar9 * ((int)local_188[5] >> 0x1f) + local_188[5] * iVar10
                                         + (int)((ulonglong)uVar8 * (ulonglong)local_188[0x19] >>
                                                0x20) + uVar8 * ((int)local_188[0x19] >> 0x1f) +
                                           local_188[0x19] * iVar11 + (uint)(uVar2 < uVar13),0x10000
                                   ,0);
    local_188[6] = local_188[0x1a];
    local_188[7] = local_11c;
    uVar2 = param_1[9];
    uVar7 = param_1[6];
    uVar13 = (uint)((ulonglong)uVar9 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar8 * (ulonglong)uVar2) + uVar13;
    local_118 = local_188[8];
    local_188[0x1a] =
         __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar8 * (ulonglong)uVar2 >> 0x20) +
                               uVar8 * ((int)uVar2 >> 0x1f) + uVar2 * iVar11 +
                               (int)((ulonglong)uVar9 * (ulonglong)uVar7 >> 0x20) +
                               uVar9 * ((int)uVar7 >> 0x1f) + uVar7 * iVar10 +
                               (uint)(uVar5 < uVar13),0x10000,0);
    local_11c = 0;
    uVar2 = param_1[0xb];
    uVar7 = param_1[8];
    uVar13 = (uint)((ulonglong)uVar9 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar8 * (ulonglong)uVar2) + uVar13;
    local_188[0x14] =
         __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar8 * (ulonglong)uVar2 >> 0x20) +
                               uVar8 * ((int)uVar2 >> 0x1f) + uVar2 * iVar11 +
                               (int)((ulonglong)uVar9 * (ulonglong)uVar7 >> 0x20) +
                               uVar9 * ((int)uVar7 >> 0x1f) + uVar7 * iVar10 +
                               (uint)(uVar5 < uVar13),0x10000,0);
    local_188[0x12] = local_188[0x1a];
    local_188[0x13] = local_11c;
    uVar13 = (uint)((ulonglong)uVar9 * (ulonglong)local_188[0x18]);
    uVar2 = (int)((ulonglong)uVar8 * (ulonglong)local_188[0x1a]) + uVar13;
    local_118 = local_188[0x14];
    local_188[0x1a] =
         __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar8 * (ulonglong)local_188[0x1a] >> 0x20) +
                               uVar8 * ((int)local_188[0x1a] >> 0x1f) + local_188[0x1a] * iVar11 +
                               (int)((ulonglong)uVar9 * (ulonglong)local_188[0x18] >> 0x20) +
                               uVar9 * ((int)local_188[0x18] >> 0x1f) + local_188[0x18] * iVar10 +
                               (uint)(uVar2 < uVar13),0x10000,0);
    uVar13 = (uint)((ulonglong)uVar9 * (ulonglong)local_188[0x19]);
    local_11c = 0;
    uVar2 = (int)((ulonglong)uVar8 * (ulonglong)local_188[0x14]) + uVar13;
    local_188[0x11] =
         __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar8 * (ulonglong)local_188[0x14] >> 0x20) +
                               uVar8 * ((int)local_188[0x14] >> 0x1f) + local_188[0x14] * iVar11 +
                               (int)((ulonglong)uVar9 * (ulonglong)local_188[0x19] >> 0x20) +
                               uVar9 * ((int)local_188[0x19] >> 0x1f) + local_188[0x19] * iVar10 +
                               (uint)(uVar2 < uVar13),0x10000,0);
    local_188[0xf] = local_188[0x1a];
    local_188[0x10] = local_11c;
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)local_188[0x1a]);
    uVar2 = (int)((ulonglong)uVar9 * (ulonglong)local_188[6]) + uVar13;
    local_118 = local_188[0x11];
    local_188[0x1a] =
         __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar9 * (ulonglong)local_188[6] >> 0x20) +
                               uVar9 * ((int)local_188[6] >> 0x1f) + local_188[6] * iVar10 +
                               (int)((ulonglong)uVar8 * (ulonglong)local_188[0x1a] >> 0x20) +
                               uVar8 * ((int)local_188[0x1a] >> 0x1f) + local_188[0x1a] * iVar11 +
                               (uint)(uVar2 < uVar13),0x10000,0);
    uVar2 = param_1[1];
    uVar7 = param_1[10];
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)uVar7);
    uVar5 = (int)((ulonglong)uVar9 * (ulonglong)uVar2) + uVar13;
    local_11c = __divdi3/*0x8004779c*/(uVar5,(int)((ulonglong)uVar9 * (ulonglong)uVar2 >> 0x20) +
                                      uVar9 * ((int)uVar2 >> 0x1f) + uVar2 * iVar10 +
                                      (int)((ulonglong)uVar8 * (ulonglong)uVar7 >> 0x20) +
                                      uVar8 * ((int)uVar7 >> 0x1f) + uVar7 * iVar11 +
                                      (uint)(uVar5 < uVar13),0x10000,0);
    uVar13 = (uint)((ulonglong)uVar8 * (ulonglong)local_188[0x11]);
    uVar2 = (int)((ulonglong)uVar9 * (ulonglong)local_188[8]) + uVar13;
    local_118 = __divdi3/*0x8004779c*/(uVar2,(int)((ulonglong)uVar9 * (ulonglong)local_188[8] >> 0x20) +
                                      uVar9 * ((int)local_188[8] >> 0x1f) + local_188[8] * iVar10 +
                                      (int)((ulonglong)uVar8 * (ulonglong)local_188[0x11] >> 0x20) +
                                      uVar8 * ((int)local_188[0x11] >> 0x1f) +
                                      local_188[0x11] * iVar11 + (uint)(uVar2 < uVar13),0x10000,0);
    local_188[0xc] = local_188[0x1a];
    local_188[0xd] = local_11c;
    local_188[0xe] = local_118;
    local_188[9] = local_188[0x1a];
    local_188[10] = local_11c;
    local_188[0xb] = local_118;
    uVar13 = uVar12;
    if (param_2 == 0) {
LAB_801043e8:
      local_30 = (longlong)(int)(local_118 - param_1[2]) * (longlong)(int)(local_118 - param_1[2]);
      iVar11 = (int)((ulonglong)((longlong)param_3 * (longlong)param_3) >> 0x20);
      lVar1 = (longlong)(int)(local_188[0x1a] - *param_1) *
              (longlong)(int)(local_188[0x1a] - *param_1) + local_30;
      iVar10 = (int)((ulonglong)lVar1 >> 0x20);
      if ((iVar11 < iVar10) ||
         ((uVar12 = uVar13, iVar10 == iVar11 &&
          ((uint)((longlong)param_3 * (longlong)param_3) < (uint)lVar1)))) {
LAB_8010446c:
        uVar12 = uVar8;
      }
    }
    else {
      local_30 = (longlong)(int)(local_118 - param_1[0xb]) *
                 (longlong)(int)(local_118 - param_1[0xb]);
      iVar11 = (int)((ulonglong)((longlong)param_3 * (longlong)param_3) >> 0x20);
      lVar1 = (longlong)(int)(local_188[0x1a] - param_1[9]) *
              (longlong)(int)(local_188[0x1a] - param_1[9]) + local_30;
      iVar10 = (int)((ulonglong)lVar1 >> 0x20);
      if (iVar10 <= iVar11) {
        if (iVar10 != iVar11) goto LAB_8010446c;
        uVar13 = uVar8;
        if ((uint)lVar1 <= (uint)((longlong)param_3 * (longlong)param_3)) goto LAB_801043e8;
      }
    }
    iVar3 = uVar8 + uVar12;
    iVar10 = uVar12 - uVar8;
    iVar11 = iVar3 - (iVar3 >> 0x1f) >> 0x1f;
    uVar8 = iVar3 / 2;
    if (iVar10 < 2) {
      puVar4 = local_188;
      if (param_2 != 0) {
        puVar6 = param_1;
        do {
          uVar8 = puVar4[1];
          uVar12 = puVar4[2];
          uVar13 = puVar4[3];
          *puVar6 = *puVar4;
          puVar6[1] = uVar8;
          puVar6[2] = uVar12;
          puVar6[3] = uVar13;
          puVar4 = puVar4 + 4;
          puVar6 = puVar6 + 4;
        } while (puVar4 != local_188 + 0xc);
      }
      puVar4 = local_188 + 0xc;
      do {
        uVar8 = puVar4[1];
        uVar12 = puVar4[2];
        uVar13 = puVar4[3];
        *param_1 = *puVar4;
        param_1[1] = uVar8;
        param_1[2] = uVar12;
        param_1[3] = uVar13;
        puVar4 = puVar4 + 4;
        param_1 = param_1 + 4;
      } while (puVar4 != local_188 + 0x18);
      return;
    }
  } while( true );
}

