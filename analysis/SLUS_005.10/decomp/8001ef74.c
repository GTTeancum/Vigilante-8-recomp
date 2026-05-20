// addr: 0x8001ef74  name: FUN_8001ef74

int FUN_8001ef74(short *param_1,MATRIX *param_2,int param_3,int *param_4,short *param_5)

{
  longlong lVar1;
  SVECTOR *r0;
  int iVar2;
  undefined **ppuVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int iVar9;
  int iVar10;
  undefined **ppuVar11;
  short *psVar12;
  int iVar13;
  undefined1 *puVar14;
  undefined1 auStack_68 [16];
  int local_58;
  int local_54;
  int local_50;
  
  puVar14 = auStack_68;
  if (param_1 == (short *)0x0) {
    return 0;
  }
  local_58 = *param_4 - param_2->t[0];
  local_54 = param_4[1] - param_2->t[1];
  local_50 = param_4[2] - param_2->t[2];
LAB_8001eff4:
  do {
    while( true ) {
      psVar12 = param_1;
      if (*psVar12 == 0) {
        return 0;
      }
      if (*psVar12 == 1) break;
      param_1 = psVar12;
      if (*psVar12 == 2) {
        SetRotMatrix(param_2);
        iVar13 = 0x7fff0000;
        ppuVar11 = &PTR_s_SunLensFlare_80010000;
        iVar9 = 0;
        if (psVar12[1] != 0) {
          iVar10 = 4;
          do {
            r0 = (SVECTOR *)((int)psVar12 + iVar10);
            gte_ldv0(r0);
            gte_rtv0();
            iVar2._0_2_ = r0[1].vx;
            iVar2._2_2_ = r0[1].vy;
            iVar4 = gte_stIR1();
            *(longlong *)(puVar14 + 0x38) = (longlong)iVar4 * (longlong)*(int *)(puVar14 + 0x10);
            iVar5 = gte_stIR3();
            *(longlong *)(puVar14 + 0x38) = (longlong)iVar5 * (longlong)*(int *)(puVar14 + 0x18);
            lVar1 = ((longlong)iVar2 * 0x1000 - (longlong)iVar4 * (longlong)*(int *)(puVar14 + 0x10)
                    ) - (longlong)iVar5 * (longlong)*(int *)(puVar14 + 0x18);
            uVar6 = (undefined4)((ulonglong)lVar1 >> 0x20);
            iVar2 = gte_stIR2();
            if (iVar2 < 0) {
              iVar2 = gte_stIR2();
              ppuVar3 = (undefined **)__divdi3((int)lVar1,uVar6,iVar2,iVar2 >> 0x1f);
              if ((int)ppuVar11 < (int)ppuVar3) {
                uVar6 = gte_stIR1();
                uVar7 = gte_stIR2();
                uVar8 = gte_stIR3();
                *(short *)(puVar14 + 0x30) = (short)uVar6;
                *(short *)(puVar14 + 0x32) = (short)uVar7;
                *(short *)(puVar14 + 0x34) = (short)uVar8;
                ppuVar11 = ppuVar3;
              }
            }
            else {
              iVar2 = gte_stIR2();
              if (iVar2 < 1) {
                if (lVar1 < 0) goto LAB_8001f358;
              }
              else {
                iVar2 = gte_stIR2();
                iVar2 = __divdi3((int)lVar1,uVar6,iVar2,iVar2 >> 0x1f);
                if (iVar2 < *(int *)(puVar14 + 0x14)) goto LAB_8001f358;
                if (iVar2 < iVar13) {
                  iVar13 = iVar2;
                }
              }
            }
            iVar9 = iVar9 + 1;
            iVar10 = iVar10 + 0xc;
          } while (iVar9 < (int)(uint)(ushort)psVar12[1]);
        }
        if (((((int)ppuVar11 < iVar13) && (iVar13 = (int)ppuVar11 + param_2->t[1], iVar13 < param_3)
             ) && (param_4[1] + -0x2800 < iVar13)) && (*(short *)(puVar14 + 0x32) < -0x800)) {
          if (param_5 != (short *)0x0) {
            uVar6 = *(undefined4 *)(puVar14 + 0x34);
            *(undefined4 *)param_5 = *(undefined4 *)(puVar14 + 0x30);
            *(undefined4 *)(param_5 + 2) = uVar6;
          }
          return param_2->t[1] + (int)ppuVar11;
        }
LAB_8001f358:
        param_1 = psVar12 + (uint)(ushort)psVar12[1] * 6 + 2;
      }
    }
    FUN_8004352c(param_2,puVar14 + 0x10,puVar14 + 0x20);
    if ((((*(int *)(psVar12 + 8) <= *(int *)(puVar14 + 0x20)) ||
         (*(int *)(puVar14 + 0x20) <= *(int *)(psVar12 + 2))) ||
        (*(int *)(psVar12 + 0xc) <= *(int *)(puVar14 + 0x28))) ||
       ((*(int *)(puVar14 + 0x28) <= *(int *)(psVar12 + 6) ||
        (iVar13 = *(int *)(puVar14 + 0x24), *(int *)(psVar12 + 10) <= iVar13)))) {
LAB_8001f178:
      param_1 = psVar12 + 0xe;
      goto LAB_8001eff4;
    }
    iVar9 = *(int *)(psVar12 + 4);
    if ((iVar9 + 0x2800 <= iVar13) || (iVar13 <= iVar9 + -0x2800)) goto LAB_8001f178;
    if ((param_2->t[1] + iVar9 < param_3) || (param_3 + 0x10000 < param_4[1])) goto LAB_8001f118;
    iVar13 = FUN_800255f4(*param_4,param_4[2]);
    param_1 = psVar12 + 0xe;
    if (*(short *)(iVar13 + 2) == 0) {
LAB_8001f118:
      if (param_5 != (short *)0x0) {
        *param_5 = -param_2->m[0][1];
        param_5[1] = -param_2->m[1][1];
        param_5[2] = -param_2->m[2][1];
      }
      *(undefined4 *)(puVar14 + 0x24) = *(undefined4 *)(psVar12 + 4);
      FUN_80043358(param_2,puVar14 + 0x20,puVar14 + 0x20);
      return param_2->t[1] + *(int *)(puVar14 + 0x24);
    }
  } while( true );
}

