// addr: 0x80040e5c  name: FUN_80040e5c

void FUN_80040e5c(int param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint *puVar6;
  int iVar7;
  int iVar8;
  uint *puVar9;
  int iVar10;
  int iVar11;
  uint *puVar12;
  SVECTOR *pSVar13;
  uint *puVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  uint uVar18;
  SVECTOR *r0;
  uint uVar19;
  uint uVar20;
  uint uVar21;
  int *piVar22;
  uint uVar23;
  undefined1 local_38 [8];
  undefined2 local_30;
  
  piVar22 = (int *)local_38;
  iVar15 = *(int *)(param_1 + 8);
  uVar23 = (uint)*(ushort *)(iVar15 + 10);
  local_30 = *(undefined2 *)(iVar15 + 0x2e);
  r0 = (SVECTOR *)(param_1 + 0x20);
  iVar16 = iRam0000060c + 0x40;
  gte_ldv0(r0);
  gte_rtps();
  gte_ldVXY0(r0 + 1);
  gte_ldVZ0(&r0[1].vz);
  gte_rtps();
  if (*(int *)(iVar15 + 0x14) <= *(int *)(iVar15 + 0x18) + *(int *)(param_1 + 0x1c)) {
    *(undefined4 *)(iVar15 + 0x18) = 0;
  }
  iVar17 = 0;
  if ((*(ushort *)(iVar15 + 0x2c) & 2) == 0) {
    uVar18 = *(int *)(iVar15 + 0xc) + *(int *)(iVar15 + 0x18) * 0x28;
    if (0 < *(int *)(param_1 + 0x1c)) {
      uVar19 = 0x9000000;
      pSVar13 = (SVECTOR *)(param_1 + 0x38);
      puVar14 = (uint *)(uVar18 - 0x28);
      do {
        gte_ldVXY0(pSVar13 + -1);
        gte_ldVZ0(&pSVar13[-1].vz);
        gte_rtps();
        gte_nclip();
        iVar7 = gte_stMAC0();
        if (iVar7 < 0) {
          iVar7 = gte_stSZ2();
          if (iVar7 < (int)(uint)*(ushort *)((int)piVar22 + 8)) {
            iVar7 = gte_stSZ1();
            if (((0 < iVar7) || (iVar7 = gte_stSZ2(), 0 < iVar7)) ||
               (iVar7 = gte_stSZ3(), 0 < iVar7)) {
              puVar12 = (uint *)(*(int *)(iVar15 + 0x10) + *(int *)(iVar15 + 0x20) * 0xa0);
              gte_stSXY0();
              gte_stSXY1();
              gte_stSXY2();
              uVar2 = gte_stSXY0();
              iVar10 = (int)(short)uVar2;
              uVar2 = gte_stSXY1();
              iVar7 = (int)(short)uVar2;
              iVar5 = iVar7;
              if (iVar10 < iVar7) {
                iVar5 = iVar10;
              }
              if (iVar7 < iVar10) {
                iVar7 = iVar10;
              }
              uVar2 = gte_stSXY2();
              iVar10 = (int)(short)uVar2;
              iVar8 = iVar10;
              if (iVar5 < iVar10) {
                iVar8 = iVar5;
              }
              if (iVar10 < iVar7) {
                iVar10 = iVar7;
              }
              iVar7 = gte_stSZ1();
              iVar3 = gte_stSZ2();
              iVar3 = iVar3 - iVar7;
              iVar4 = gte_stSZ3();
              iVar4 = iVar4 - iVar7;
              iVar11 = iVar7 * 4 + iVar3 + iVar4;
              gte_ldv0(pSVar13);
              gte_ldVXY1((((int)r0->vx + (int)pSVar13[-2].vx) / 2 & 0xffffU) +
                         (((int)pSVar13[-3].vy + (int)pSVar13[-2].vy) / 2) * 0x10000);
              gte_ldVZ1(((int)pSVar13[-3].vz + (int)pSVar13[-2].vz) / 2);
              gte_ldVXY2((((int)r0->vx + (int)pSVar13[-1].vx) / 2 & 0xffffU) +
                         (((int)pSVar13[-3].vy + (int)pSVar13[-1].vy) / 2) * 0x10000);
              gte_ldVZ2(((int)pSVar13[-3].vz + (int)pSVar13[-1].vz) / 2);
              gte_rtpt();
              uVar1 = (undefined1)pSVar13[-3].pad;
              *(undefined1 *)(puVar12 + 1) = uVar1;
              *(undefined1 *)((int)puVar12 + 5) = uVar1;
              *(undefined1 *)((int)puVar12 + 6) = uVar1;
              uVar2 = gte_stSXY0();
              iVar7 = (int)(short)uVar2;
              iVar5 = iVar7;
              if (iVar8 < iVar7) {
                iVar5 = iVar8;
              }
              if (iVar5 < 0x140) {
                if (iVar7 < iVar10) {
                  iVar7 = iVar10;
                }
                if (-1 < iVar7) {
                  uVar20 = gte_stSXY0();
                  puVar12[0x26] = uVar20;
                  uVar20 = gte_stSXY1();
                  puVar12[0xc] = uVar20;
                  puVar12[4] = uVar20;
                  uVar20 = gte_stSXY2();
                  puVar6 = (uint *)((iVar11 >> 5) * 4 + iVar16);
                  puVar12[0x16] = uVar20;
                  puVar12[6] = uVar20;
                  uVar20 = *puVar6;
                  iVar7 = iVar11 + iVar3 * 2;
                  *puVar6 = (uint)puVar12 & 0xffffff;
                  puVar6 = (uint *)((iVar7 >> 5) * 4 + iVar16);
                  *puVar12 = uVar20 | uVar19;
                  uVar20 = *puVar6;
                  *puVar6 = (uint)(puVar12 + 10) & 0xffffff;
                  puVar6 = (uint *)((iVar11 + iVar4 * 2 >> 5) * 4 + iVar16);
                  puVar12[10] = uVar20 | uVar19;
                  uVar20 = *puVar6;
                  puVar9 = (uint *)((iVar7 + iVar4 * 2 >> 5) * 4 + iVar16);
                  *puVar6 = (uint)(puVar12 + 0x14) & 0xffffff;
                  puVar12[0x14] = uVar20 | uVar19;
                  uVar20 = *puVar9;
                  *puVar9 = (uint)(puVar12 + 0x1e) & 0xffffff;
                  puVar12[0x1e] = uVar20 | uVar19;
                  gte_ldVXY0((((int)pSVar13[-2].vx + (int)pSVar13->vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-2].vy + (int)pSVar13->vy) / 2) * 0x10000);
                  gte_ldVZ0(((int)pSVar13[-2].vz + (int)pSVar13->vz) / 2);
                  gte_ldVXY1((((int)pSVar13[-1].vx + (int)pSVar13->vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-1].vy + (int)pSVar13->vy) / 2) * 0x10000);
                  gte_ldVZ1(((int)pSVar13[-1].vz + (int)pSVar13->vz) / 2);
                  gte_ldVXY2((((int)pSVar13[-2].vx + (int)pSVar13[-1].vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-2].vy + (int)pSVar13[-1].vy) / 2) * 0x10000);
                  gte_ldVZ2(((int)pSVar13[-2].vz + (int)pSVar13[-1].vz) / 2);
                  gte_rtpt();
                  uVar20 = puVar12[1];
                  puVar12[0x1f] = uVar20;
                  puVar12[0x15] = uVar20;
                  puVar12[0xb] = uVar20;
                  uVar20 = gte_stSXY0();
                  puVar12[0x22] = uVar20;
                  puVar12[0x12] = uVar20;
                  uVar20 = gte_stSXY1();
                  puVar12[0x24] = uVar20;
                  puVar12[0x1c] = uVar20;
                  uVar20 = gte_stSXY2();
                  puVar12[0x20] = uVar20;
                  puVar12[0x18] = uVar20;
                  puVar12[0x10] = uVar20;
                  puVar12[8] = uVar20;
                  iVar5 = *(int *)(iVar15 + 0x20) + 1;
                  iVar7 = 0;
                  if (iVar5 != *(int *)(iVar15 + 0x1c)) {
                    iVar7 = iVar5;
                  }
                  *(int *)(iVar15 + 0x20) = iVar7;
                }
              }
              gte_ldVXY0(pSVar13 + -2);
              gte_ldVZ0(&pSVar13[-2].vz);
              gte_ldVXY1(pSVar13 + -1);
              gte_ldVZ1(&pSVar13[-1].vz);
              gte_ldv2(pSVar13);
              gte_rtpt();
            }
          }
          else {
            iVar7 = gte_stFLAG();
            if (-1 < iVar7) {
              gte_stSXY0();
              gte_stSXY1();
              gte_stSXY2();
              gte_ldv0(pSVar13);
              gte_rtps();
              uVar1 = (undefined1)pSVar13[-3].pad;
              *(undefined1 *)(puVar14 + 0xb) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x2d) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x2e) = uVar1;
              gte_stSXY2();
              gte_avsz4();
              uVar2 = gte_stIR0();
              *(short *)((int)puVar14 + 0x36) = (short)uVar23 + (short)((uint)uVar2 >> 8) * 0x40;
              uVar20 = gte_stOTZ();
              puVar12 = (uint *)((uVar20 >> 1) * 4 + iVar16);
              uVar20 = *puVar12;
              puVar14 = puVar14 + 10;
              *puVar12 = uVar18 & 0xffffff;
              *puVar14 = uVar20 | uVar19;
              uVar18 = uVar18 + 0x28;
              *(int *)(iVar15 + 0x18) = *(int *)(iVar15 + 0x18) + 1;
            }
          }
        }
        else {
          gte_ldv0(pSVar13);
          gte_rtps();
        }
        iVar17 = iVar17 + 1;
        pSVar13 = pSVar13 + 2;
        r0 = r0 + 2;
      } while (iVar17 < *(int *)(param_1 + 0x1c));
    }
  }
  else {
    iVar7 = *(int *)(param_1 + 0x1c);
    uVar18 = *(int *)(iVar15 + 0xc) + *(int *)(iVar15 + 0x18) * 0x34;
    *piVar22 = *(undefined4 *)(iVar15 + 0x10);
    if (0 < iVar7) {
      uVar20 = 0xc000000;
      uVar19 = 0x3c000000;
      pSVar13 = (SVECTOR *)(param_1 + 0x38);
      puVar14 = (uint *)(uVar18 - 0x34);
      do {
        gte_ldVXY0(pSVar13 + -1);
        gte_ldVZ0(&pSVar13[-1].vz);
        gte_rtps();
        gte_nclip();
        iVar7 = gte_stMAC0();
        if (iVar7 < 0) {
          iVar7 = gte_stSZ2();
          if (iVar7 < (int)(uint)*(ushort *)(piVar22 + 2)) {
            iVar7 = gte_stSZ1();
            if (((0 < iVar7) || (iVar7 = gte_stSZ2(), 0 < iVar7)) ||
               (iVar7 = gte_stSZ3(), 0 < iVar7)) {
              puVar12 = (uint *)(*piVar22 + *(int *)(iVar15 + 0x20) * 0xd0);
              gte_stSXY0();
              gte_stSXY1();
              gte_stSXY2();
              uVar2 = gte_stSXY0();
              iVar10 = (int)(short)uVar2;
              uVar2 = gte_stSXY1();
              iVar7 = (int)(short)uVar2;
              iVar5 = iVar7;
              if (iVar10 < iVar7) {
                iVar5 = iVar10;
              }
              if (iVar7 < iVar10) {
                iVar7 = iVar10;
              }
              uVar2 = gte_stSXY2();
              iVar10 = (int)(short)uVar2;
              iVar8 = iVar10;
              if (iVar5 < iVar10) {
                iVar8 = iVar5;
              }
              if (iVar10 < iVar7) {
                iVar10 = iVar7;
              }
              iVar7 = gte_stSZ1();
              iVar3 = gte_stSZ2();
              iVar3 = iVar3 - iVar7;
              iVar4 = gte_stSZ3();
              iVar4 = iVar4 - iVar7;
              iVar11 = iVar7 * 4 + iVar3 + iVar4;
              gte_ldv0(pSVar13);
              gte_ldVXY1((((int)r0->vx + (int)pSVar13[-2].vx) / 2 & 0xffffU) +
                         (((int)pSVar13[-3].vy + (int)pSVar13[-2].vy) / 2) * 0x10000);
              gte_ldVZ1(((int)pSVar13[-3].vz + (int)pSVar13[-2].vz) / 2);
              gte_ldVXY2((((int)r0->vx + (int)pSVar13[-1].vx) / 2 & 0xffffU) +
                         (((int)pSVar13[-3].vy + (int)pSVar13[-1].vy) / 2) * 0x10000);
              gte_ldVZ2(((int)pSVar13[-3].vz + (int)pSVar13[-1].vz) / 2);
              gte_rtpt();
              uVar1 = (undefined1)pSVar13[-3].pad;
              *(undefined1 *)(puVar12 + 1) = uVar1;
              *(undefined1 *)((int)puVar12 + 5) = uVar1;
              *(undefined1 *)((int)puVar12 + 6) = uVar1;
              uVar1 = (undefined1)pSVar13[-2].pad;
              *(undefined1 *)(puVar12 + 0x11) = uVar1;
              *(undefined1 *)((int)puVar12 + 0x45) = uVar1;
              *(undefined1 *)((int)puVar12 + 0x46) = uVar1;
              uVar1 = (undefined1)pSVar13[-1].pad;
              *(undefined1 *)(puVar12 + 0x21) = uVar1;
              *(undefined1 *)((int)puVar12 + 0x85) = uVar1;
              *(undefined1 *)((int)puVar12 + 0x86) = uVar1;
              uVar1 = (undefined1)pSVar13->pad;
              *(undefined1 *)(puVar12 + 0x31) = uVar1;
              *(undefined1 *)((int)puVar12 + 0xc5) = uVar1;
              *(undefined1 *)((int)puVar12 + 0xc6) = uVar1;
              uVar2 = gte_stSXY0();
              iVar7 = (int)(short)uVar2;
              iVar5 = iVar7;
              if (iVar8 < iVar7) {
                iVar5 = iVar8;
              }
              if (iVar5 < 0x140) {
                if (iVar7 < iVar10) {
                  iVar7 = iVar10;
                }
                if (-1 < iVar7) {
                  uVar21 = gte_stSXY0();
                  puVar12[0x32] = uVar21;
                  uVar21 = gte_stSXY1();
                  puVar12[0xf] = uVar21;
                  puVar12[5] = uVar21;
                  uVar21 = gte_stSXY2();
                  puVar6 = (uint *)((iVar11 >> 5) * 4 + iVar16);
                  puVar12[0x1c] = uVar21;
                  puVar12[8] = uVar21;
                  uVar21 = *puVar6;
                  iVar7 = iVar11 + iVar3 * 2;
                  *puVar6 = (uint)puVar12 & 0xffffff;
                  puVar6 = (uint *)((iVar7 >> 5) * 4 + iVar16);
                  *puVar12 = uVar21 | uVar20;
                  uVar21 = *puVar6;
                  *puVar6 = (uint)(puVar12 + 0xd) & 0xffffff;
                  puVar6 = (uint *)((iVar11 + iVar4 * 2 >> 5) * 4 + iVar16);
                  puVar12[0xd] = uVar21 | uVar20;
                  uVar21 = *puVar6;
                  puVar9 = (uint *)((iVar7 + iVar4 * 2 >> 5) * 4 + iVar16);
                  *puVar6 = (uint)(puVar12 + 0x1a) & 0xffffff;
                  puVar12[0x1a] = uVar21 | uVar20;
                  uVar21 = *puVar9;
                  *puVar9 = (uint)(puVar12 + 0x27) & 0xffffff;
                  puVar12[0x27] = uVar21 | uVar20;
                  gte_ldVXY0((((int)pSVar13[-2].vx + (int)pSVar13->vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-2].vy + (int)pSVar13->vy) / 2) * 0x10000);
                  gte_ldVZ0(((int)pSVar13[-2].vz + (int)pSVar13->vz) / 2);
                  gte_ldVXY1((((int)pSVar13[-1].vx + (int)pSVar13->vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-1].vy + (int)pSVar13->vy) / 2) * 0x10000);
                  gte_ldVZ1(((int)pSVar13[-1].vz + (int)pSVar13->vz) / 2);
                  gte_ldVXY2((((int)pSVar13[-2].vx + (int)pSVar13[-1].vx) / 2 & 0xffffU) +
                             (((int)pSVar13[-2].vy + (int)pSVar13[-1].vy) / 2) * 0x10000);
                  gte_ldVZ2(((int)pSVar13[-2].vz + (int)pSVar13[-1].vz) / 2);
                  gte_rtpt();
                  uVar21 = (((int)pSVar13[-3].pad + (int)pSVar13[-2].pad) / 2) * 0x10101 | uVar19;
                  puVar12[0xe] = uVar21;
                  puVar12[4] = uVar21;
                  uVar21 = (((int)pSVar13[-3].pad + (int)pSVar13[-1].pad) / 2) * 0x10101 | uVar19;
                  puVar12[0x1b] = uVar21;
                  puVar12[7] = uVar21;
                  uVar21 = (((int)pSVar13[-1].pad + (int)pSVar13->pad) / 2) * 0x10101 | uVar19;
                  puVar12[0x2e] = uVar21;
                  puVar12[0x24] = uVar21;
                  uVar21 = (((int)pSVar13[-2].pad + (int)pSVar13->pad) / 2) * 0x10101 | uVar19;
                  puVar12[0x2b] = uVar21;
                  puVar12[0x17] = uVar21;
                  uVar21 = (((int)pSVar13[-2].pad + (int)pSVar13[-1].pad) / 2) * 0x10101 | uVar19;
                  puVar12[0x28] = uVar21;
                  puVar12[0x1e] = uVar21;
                  puVar12[0x14] = uVar21;
                  puVar12[10] = uVar21;
                  uVar21 = gte_stSXY0();
                  puVar12[0x2c] = uVar21;
                  puVar12[0x18] = uVar21;
                  uVar21 = gte_stSXY1();
                  puVar12[0x2f] = uVar21;
                  puVar12[0x25] = uVar21;
                  uVar21 = gte_stSXY2();
                  puVar12[0x29] = uVar21;
                  puVar12[0x1f] = uVar21;
                  puVar12[0x15] = uVar21;
                  puVar12[0xb] = uVar21;
                  iVar5 = *(int *)(iVar15 + 0x20) + 1;
                  iVar7 = 0;
                  if (iVar5 != *(int *)(iVar15 + 0x1c)) {
                    iVar7 = iVar5;
                  }
                  *(int *)(iVar15 + 0x20) = iVar7;
                }
              }
              gte_ldVXY0(pSVar13 + -2);
              gte_ldVZ0(&pSVar13[-2].vz);
              gte_ldVXY1(pSVar13 + -1);
              gte_ldVZ1(&pSVar13[-1].vz);
              gte_ldv2(pSVar13);
              gte_rtpt();
            }
          }
          else {
            iVar7 = gte_stFLAG();
            if (-1 < iVar7) {
              gte_stSXY0();
              gte_stSXY1();
              gte_stSXY2();
              gte_ldv0(pSVar13);
              gte_rtps();
              uVar1 = (undefined1)pSVar13[-3].pad;
              *(undefined1 *)(puVar14 + 0xe) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x39) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x3a) = uVar1;
              uVar1 = (undefined1)pSVar13[-2].pad;
              *(undefined1 *)(puVar14 + 0x11) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x45) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x46) = uVar1;
              uVar1 = (undefined1)pSVar13[-1].pad;
              *(undefined1 *)(puVar14 + 0x14) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x51) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x52) = uVar1;
              uVar1 = (undefined1)pSVar13->pad;
              *(undefined1 *)(puVar14 + 0x17) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x5d) = uVar1;
              *(undefined1 *)((int)puVar14 + 0x5e) = uVar1;
              gte_stSXY2();
              gte_avsz4();
              uVar2 = gte_stIR0();
              *(short *)((int)puVar14 + 0x42) = (short)uVar23 + (short)((uint)uVar2 >> 8) * 0x40;
              uVar21 = gte_stOTZ();
              puVar12 = (uint *)((uVar21 >> 1) * 4 + iVar16);
              uVar21 = *puVar12;
              puVar14 = puVar14 + 0xd;
              *puVar12 = uVar18 & 0xffffff;
              *puVar14 = uVar21 | uVar20;
              uVar18 = uVar18 + 0x34;
              *(int *)(iVar15 + 0x18) = *(int *)(iVar15 + 0x18) + 1;
            }
          }
        }
        else {
          gte_ldv0(pSVar13);
          gte_rtps();
        }
        iVar17 = iVar17 + 1;
        pSVar13 = pSVar13 + 2;
        r0 = r0 + 2;
      } while (iVar17 < *(int *)(param_1 + 0x1c));
    }
  }
  return;
}

