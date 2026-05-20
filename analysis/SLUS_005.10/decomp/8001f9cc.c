// addr: 0x8001f9cc  name: FUN_8001f9cc

undefined4 FUN_8001f9cc(int param_1,short param_2)

{
  short sVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  short *psVar5;
  uint in_t0;
  uint uVar6;
  uint uVar7;
  uint *puVar8;
  uint *puVar9;
  uint uVar10;
  uint *puVar11;
  
  if (*(ushort **)(param_1 + 0x60) != (ushort *)0x0) {
    if ((ushort)(param_2 - *(short *)(param_1 + 0x46)) < **(ushort **)(param_1 + 0x60)) {
      return 0;
    }
    bVar2 = false;
    puVar11 = (uint *)0x0;
    do {
      psVar5 = *(short **)(param_1 + 0x60);
      uVar10 = (uint)psVar5[1];
      puVar8 = (uint *)(psVar5 + 2);
      if ((int)uVar10 < 0) {
        *(short *)(param_1 + 0x46) = *(short *)(param_1 + 0x46) + *psVar5;
        *(uint *)(param_1 + 0x60) = *(int *)(param_1 + 0x60) + uVar10;
        if (*(code **)(param_1 + 100) == (code *)0x0) {
          iVar3 = 0;
        }
        else {
          iVar3 = (**(code **)(param_1 + 100))(param_1,5,0);
        }
        if (iVar3 < 0) {
          return 0xffffffff;
        }
      }
      else {
        if ((uVar10 & 1) != 0) {
          uVar6 = (int)psVar5 + 7U & 3;
          uVar7 = (uint)(psVar5 + 2) & 3;
          in_t0 = (*(int *)(((int)psVar5 + 7U) - uVar6) << (3 - uVar6) * 8 |
                  in_t0 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar7) * 8 |
                  *(uint *)((int)(psVar5 + 2) - uVar7) >> uVar7 * 8;
          sVar1 = psVar5[4];
          uVar6 = param_1 + 0x43U & 3;
          puVar8 = (uint *)((param_1 + 0x43U) - uVar6);
          *puVar8 = *puVar8 & -1 << (uVar6 + 1) * 8 | in_t0 >> (3 - uVar6) * 8;
          uVar6 = param_1 + 0x40U & 3;
          puVar8 = (uint *)((param_1 + 0x40U) - uVar6);
          *puVar8 = *puVar8 & 0xffffffffU >> (4 - uVar6) * 8 | in_t0 << uVar6 * 8;
          *(short *)(param_1 + 0x44) = sVar1;
          puVar8 = (uint *)(psVar5 + 6);
          bVar2 = true;
        }
        if ((uVar10 & 2) != 0) {
          in_t0 = *puVar8;
          uVar6 = puVar8[1];
          uVar7 = puVar8[2];
          *(uint *)(param_1 + 0x48) = in_t0;
          *(uint *)(param_1 + 0x4c) = uVar6;
          *(uint *)(param_1 + 0x50) = uVar7;
          puVar8 = puVar8 + 3;
          bVar2 = true;
        }
        if ((uVar10 & 8) != 0) {
          *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + (int)(short)(ushort)*puVar8;
          *(int *)(param_1 + 0x4c) =
               *(int *)(param_1 + 0x4c) + (int)(short)*(ushort *)((int)puVar8 + 2);
          puVar9 = puVar8 + 1;
          bVar2 = true;
          puVar8 = puVar8 + 2;
          *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + (int)(short)(ushort)*puVar9;
        }
        if ((uVar10 & 0x10) != 0) {
          do {
            uVar6 = *puVar8;
            uVar4 = FUN_8001b36c(*(undefined4 *)(param_1 + 0x58),
                                 *(short *)(*(int *)(param_1 + 0x30) + 0x2a) +
                                 *(ushort *)((int)puVar8 + 2));
            puVar8 = puVar8 + 1;
            *(undefined4 *)(*(int *)(param_1 + 0x30) + ((ushort)uVar6 & 0x7fff) * 4 + 0x2c) = uVar4;
          } while (-1 < (int)((uint)(ushort)uVar6 << 0x10));
        }
        puVar9 = puVar8;
        if ((uVar10 & 0x20) != 0) {
          puVar9 = puVar8 + 2;
          bVar2 = true;
          puVar11 = puVar8;
        }
        if ((uVar10 & 0x40) != 0) {
          *(uint **)(*(int *)(param_1 + 0x30) + 8) = puVar9 + 1;
          puVar9 = puVar9 + *puVar9 * 2 + 1;
        }
        *(uint **)(param_1 + 0x60) = puVar9;
      }
    } while (**(ushort **)(param_1 + 0x60) <= (ushort)(param_2 - *(short *)(param_1 + 0x46)));
    if (!bVar2) {
      return 0;
    }
    FUN_8001d708(param_1);
    if (puVar11 != (uint *)0x0) {
      FUN_80043864(param_1 + 0x10,puVar11,param_1 + 0x10);
      *(ushort *)(param_1 + 0x22) = (ushort)*puVar11;
    }
  }
  return 0;
}

