// addr: 0x8010246c  name: FUN_8010246c

void FUN_8010246c(int param_1)

{
  short sVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined1 uVar8;
  short *psVar9;
  undefined4 *puVar10;
  int iVar11;
  short sVar12;
  undefined4 *puVar13;
  undefined4 *puVar14;
  undefined4 uVar15;
  uint uVar16;
  undefined4 uVar17;
  short local_1f0 [2];
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined2 local_1e4;
  undefined2 uStack_1e2;
  short local_1d8;
  undefined2 local_1d6;
  undefined2 local_1cc;
  short local_1c0 [4];
  undefined4 local_1b8 [53];
  byte bStack_e1;
  undefined2 local_dc;
  undefined2 uStack_da;
  short local_d4;
  undefined2 uStack_d2;
  undefined2 local_cc;
  undefined4 uStack_c8;
  short local_c4;
  undefined4 local_c0 [40];
  
  iVar3 = *(int *)(param_1 + 0x28) >> 8;
  iVar4 = *(int *)(param_1 + 0x24) >> 8;
  sVar2 = SquareRoot0/*0x8004c6e4*/(iVar3 * iVar3 + iVar4 * iVar4);
  *(short *)(param_1 + 0x30) = sVar2 + -0x80;
  *(undefined4 *)(param_1 + 0x20) = 0;
  *(undefined4 *)(param_1 + 0x18) = 0;
  if ((*(ushort *)(param_1 + 0x2c) & 2) != 0) {
    uVar5 = Heap_AllocOrRetry/*0x800116f4*/(*(int *)(param_1 + 0x14) * 0x34);
    *(undefined4 *)(param_1 + 0xc) = uVar5;
    iVar3 = Heap_AllocOrRetry/*0x800116f4*/(*(int *)(param_1 + 0x1c) * 0xd0);
    *(int *)(param_1 + 0x10) = iVar3;
    if ((*(ushort *)(param_1 + 0x2c) & 0x100) != 0) {
      local_1ec._3_1_ = local_1ec._3_1_ | 2;
    }
    uStack_1e2 = *(undefined2 *)(param_1 + 10);
    local_1d6 = *(undefined2 *)(param_1 + 8);
    sVar2 = *(short *)(param_1 + 6);
    iVar4 = 0;
    sVar12 = sVar2 + (*(short *)(param_1 + 4) + -1) * 0x100;
    local_1e4 = sVar12;
    sVar12 = local_1e4;
    local_1d8 = sVar2 + *(short *)(param_1 + 2) + -1 + (*(short *)(param_1 + 4) + -1) * 0x100;
    sVar1 = local_1d8;
    local_1cc = sVar2;
    sVar2 = local_1cc;
    local_1c0[0] = sVar2 + *(short *)(param_1 + 2) + -1;
    if (0 < *(int *)(param_1 + 0x14)) {
      iVar11 = 0;
      do {
        psVar9 = local_1f0;
        puVar6 = (undefined4 *)(iVar11 + *(int *)(param_1 + 0xc));
        do {
          uVar5 = *(undefined4 *)(psVar9 + 2);
          uVar15 = *(undefined4 *)(psVar9 + 4);
          uVar17 = *(undefined4 *)(psVar9 + 6);
          *puVar6 = *(undefined4 *)psVar9;
          puVar6[1] = uVar5;
          puVar6[2] = uVar15;
          puVar6[3] = uVar17;
          psVar9 = psVar9 + 8;
          puVar6 = puVar6 + 4;
        } while (psVar9 != local_1c0);
        *puVar6 = *(undefined4 *)psVar9;
        iVar4 = iVar4 + 1;
        iVar11 = iVar11 + 0x34;
      } while (iVar4 < *(int *)(param_1 + 0x14));
    }
    uVar16 = 0;
    iVar4 = 0;
    puVar6 = local_1b8;
    do {
      local_1e4._0_1_ = (byte)sVar12;
      local_1e4._1_1_ = (byte)((ushort)sVar12 >> 8);
      local_1cc._1_1_ = (byte)((ushort)sVar2 >> 8);
      puVar13 = (undefined4 *)((int)local_1b8 + iVar4);
      psVar9 = local_1f0;
      do {
        uVar5 = *(undefined4 *)(psVar9 + 2);
        uVar15 = *(undefined4 *)(psVar9 + 4);
        uVar17 = *(undefined4 *)(psVar9 + 6);
        *puVar13 = *(undefined4 *)psVar9;
        puVar13[1] = uVar5;
        puVar13[2] = uVar15;
        puVar13[3] = uVar17;
        psVar9 = psVar9 + 8;
        puVar13 = puVar13 + 4;
      } while (psVar9 != local_1c0);
      *puVar13 = *(undefined4 *)psVar9;
      puVar13 = puVar6;
      if ((uVar16 & 1) == 0) {
        puVar13 = (undefined4 *)0x1;
        *(byte *)(puVar6 + 9) = (byte)local_1e4;
      }
      uVar8 = (undefined1)((int)((uint)(byte)local_1e4 + (uint)(byte)local_1d8) >> 1);
      *(undefined1 *)(puVar6 + 9) = uVar8;
      *(undefined1 *)(puVar6 + 3) = *(undefined1 *)(puVar13 + 9);
      puVar13 = puVar6;
      if ((uVar16 & 1) != 0) {
        puVar13 = (undefined4 *)0x1;
        *(byte *)(puVar6 + 0xc) = (byte)local_1d8;
      }
      *(undefined1 *)(puVar6 + 0xc) = uVar8;
      *(undefined1 *)(puVar6 + 6) = *(undefined1 *)(puVar13 + 0xc);
      puVar13 = puVar6;
      if ((uVar16 & 2) == 0) {
        puVar13 = (undefined4 *)0x1;
        *(byte *)((int)puVar6 + 0x19) = local_1e4._1_1_;
      }
      uVar8 = (undefined1)((int)((uint)local_1e4._1_1_ + (uint)local_1cc._1_1_) >> 1);
      *(undefined1 *)((int)puVar6 + 0x19) = uVar8;
      *(undefined1 *)((int)puVar6 + 0xd) = *(undefined1 *)((int)puVar13 + 0x19);
      puVar13 = puVar6;
      if ((uVar16 & 2) != 0) {
        puVar13 = (undefined4 *)0x1;
        *(byte *)((int)puVar6 + 0x31) = local_1cc._1_1_;
      }
      *(undefined1 *)((int)puVar6 + 0x31) = uVar8;
      uVar16 = uVar16 + 1;
      *(undefined1 *)((int)puVar6 + 0x25) = *(undefined1 *)((int)puVar13 + 0x31);
      iVar4 = iVar4 + 0x34;
      puVar6 = puVar6 + 0xd;
    } while ((int)uVar16 < 4);
    iVar4 = 0;
    if (*(int *)(param_1 + 0x1c) < 1) {
      return;
    }
    iVar11 = 0;
    do {
      puVar13 = (undefined4 *)(iVar11 + iVar3);
      puVar6 = local_1b8;
      do {
        uVar5 = puVar6[1];
        uVar15 = puVar6[2];
        uVar17 = puVar6[3];
        *puVar13 = *puVar6;
        puVar13[1] = uVar5;
        puVar13[2] = uVar15;
        puVar13[3] = uVar17;
        puVar6 = puVar6 + 4;
        puVar13 = puVar13 + 4;
      } while (puVar6 != local_1b8 + 0x34);
      iVar4 = iVar4 + 1;
      iVar11 = iVar11 + 0xd0;
      local_1e4 = sVar12;
      local_1d8 = sVar1;
      local_1cc = sVar2;
    } while (iVar4 < *(int *)(param_1 + 0x1c));
  }
  uVar5 = Heap_AllocOrRetry/*0x800116f4*/(*(int *)(param_1 + 0x14) * 0x28);
  *(undefined4 *)(param_1 + 0xc) = uVar5;
  uVar5 = Heap_AllocOrRetry/*0x800116f4*/(*(int *)(param_1 + 0x1c) * 0xa0);
  *(undefined4 *)(param_1 + 0x10) = uVar5;
  if ((*(ushort *)(param_1 + 0x2c) & 0x100) != 0) {
    bStack_e1 = bStack_e1 | 2;
  }
  uStack_da = *(undefined2 *)(param_1 + 10);
  uStack_d2 = *(undefined2 *)(param_1 + 8);
  sVar2 = *(short *)(param_1 + 6);
  iVar3 = 0;
  sVar12 = sVar2 + (*(short *)(param_1 + 4) + -1) * 0x100;
  local_dc = sVar12;
  local_d4 = sVar2 + *(short *)(param_1 + 2) + -1 + (*(short *)(param_1 + 4) + -1) * 0x100;
  local_cc = sVar2;
  local_c4 = sVar2 + *(short *)(param_1 + 2) + -1;
  if (0 < *(int *)(param_1 + 0x14)) {
    iVar4 = 0;
    do {
      puVar6 = (undefined4 *)(iVar4 + *(int *)(param_1 + 0xc));
      puVar13 = local_1b8 + 0x34;
      do {
        puVar10 = puVar13;
        puVar7 = puVar6;
        uVar5 = puVar10[1];
        uVar15 = puVar10[2];
        uVar17 = puVar10[3];
        *puVar7 = *puVar10;
        puVar7[1] = uVar5;
        puVar7[2] = uVar15;
        puVar7[3] = uVar17;
        puVar13 = puVar10 + 4;
        puVar6 = puVar7 + 4;
      } while (puVar13 != &uStack_c8);
      uVar5 = puVar10[5];
      puVar7[4] = *puVar13;
      puVar7[5] = uVar5;
      iVar3 = iVar3 + 1;
      iVar4 = iVar4 + 0x28;
    } while (iVar3 < *(int *)(param_1 + 0x14));
  }
  uVar16 = 0;
  iVar3 = 0;
  puVar6 = local_c0;
  do {
    local_dc._0_1_ = (byte)sVar12;
    local_dc._1_1_ = (byte)((ushort)sVar12 >> 8);
    local_cc._1_1_ = (byte)((ushort)sVar2 >> 8);
    puVar13 = local_1b8 + 0x34;
    puVar7 = (undefined4 *)((int)local_c0 + iVar3);
    do {
      puVar14 = puVar7;
      puVar10 = puVar13;
      uVar5 = puVar10[1];
      uVar15 = puVar10[2];
      uVar17 = puVar10[3];
      *puVar14 = *puVar10;
      puVar14[1] = uVar5;
      puVar14[2] = uVar15;
      puVar14[3] = uVar17;
      puVar13 = puVar10 + 4;
      puVar7 = puVar14 + 4;
    } while (puVar13 != &uStack_c8);
    uVar5 = puVar10[5];
    puVar14[4] = *puVar13;
    puVar14[5] = uVar5;
    puVar13 = puVar6;
    if ((uVar16 & 1) == 0) {
      puVar13 = (undefined4 *)0x1;
      *(byte *)(puVar6 + 7) = (byte)local_dc;
    }
    uVar8 = (undefined1)((int)((uint)(byte)local_dc + (uint)(byte)local_d4) >> 1);
    *(undefined1 *)(puVar6 + 7) = uVar8;
    *(undefined1 *)(puVar6 + 3) = *(undefined1 *)(puVar13 + 7);
    puVar13 = puVar6;
    if ((uVar16 & 1) != 0) {
      puVar13 = (undefined4 *)0x1;
      *(byte *)(puVar6 + 9) = (byte)local_d4;
    }
    *(undefined1 *)(puVar6 + 9) = uVar8;
    *(undefined1 *)(puVar6 + 5) = *(undefined1 *)(puVar13 + 9);
    puVar13 = puVar6;
    if ((uVar16 & 2) == 0) {
      puVar13 = (undefined4 *)0x1;
      *(byte *)((int)puVar6 + 0x15) = local_dc._1_1_;
    }
    uVar8 = (undefined1)((int)((uint)local_dc._1_1_ + (uint)local_cc._1_1_) >> 1);
    *(undefined1 *)((int)puVar6 + 0x15) = uVar8;
    *(undefined1 *)((int)puVar6 + 0xd) = *(undefined1 *)((int)puVar13 + 0x15);
    puVar13 = puVar6;
    if ((uVar16 & 2) != 0) {
      puVar13 = (undefined4 *)0x1;
      *(byte *)((int)puVar6 + 0x25) = local_cc._1_1_;
    }
    *(undefined1 *)((int)puVar6 + 0x25) = uVar8;
    uVar16 = uVar16 + 1;
    *(undefined1 *)((int)puVar6 + 0x1d) = *(undefined1 *)((int)puVar13 + 0x25);
    iVar3 = iVar3 + 0x28;
    puVar6 = puVar6 + 10;
  } while ((int)uVar16 < 4);
  iVar3 = 0;
  if (0 < *(int *)(param_1 + 0x1c)) {
    iVar4 = 0;
    do {
      puVar13 = local_c0;
      puVar6 = (undefined4 *)(iVar4 + *(int *)(param_1 + 0x10));
      do {
        uVar5 = puVar13[1];
        uVar15 = puVar13[2];
        uVar17 = puVar13[3];
        *puVar6 = *puVar13;
        puVar6[1] = uVar5;
        puVar6[2] = uVar15;
        puVar6[3] = uVar17;
        puVar13 = puVar13 + 4;
        puVar6 = puVar6 + 4;
      } while (puVar13 != (undefined4 *)&stack0xffffffe0);
      iVar3 = iVar3 + 1;
      iVar4 = iVar4 + 0xa0;
    } while (iVar3 < *(int *)(param_1 + 0x1c));
  }
  return;
}

