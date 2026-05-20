// addr: 0x80019458  name: FUN_80019458

undefined4 * FUN_80019458(int *param_1,byte *param_2,int param_3,short param_4)

{
  byte bVar1;
  byte bVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  byte bVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
  int iVar10;
  uint uVar11;
  byte *pbVar12;
  short *psVar13;
  short *psVar14;
  short sVar15;
  uint *p1;
  uint *p0;
  short sVar16;
  uint uVar17;
  
  uVar17 = param_1[1];
  sVar3 = *(short *)((int)param_1 + 0xe);
  iVar7 = *param_1;
  sVar4 = *(short *)((int)param_1 + 0x12);
  bVar1 = *(byte *)(iVar7 + 6);
  iVar8 = FUN_80052544(param_2);
  puVar9 = (undefined4 *)FUN_800116f4(iVar8 * 0x14 + 0xc);
  p0 = puVar9 + 1;
  *(undefined1 *)((int)puVar9 + 7) = 1;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  puVar9[2] = *(ushort *)(param_1 + 4) & 0x9ff | 0xe1000420;
  bVar6 = *param_2;
  p1 = puVar9 + 3;
  if (bVar6 != 0) {
    psVar14 = (short *)((int)puVar9 + 0x16);
    iVar8 = param_3;
    sVar16 = param_4;
    do {
      pbVar12 = param_2 + 1;
      bVar2 = *(byte *)(*param_1 + 5);
      sVar15 = (short)iVar8;
      if ((bVar2 <= bVar6) && (bVar6 = bVar6 - bVar2, bVar6 < *(byte *)(*param_1 + 4))) {
        *(uint *)(psVar14 + -3) = uVar17;
        *p1 = 0x4000000;
        psVar14[2] = sVar4;
        psVar14[4] = (ushort)bVar1;
        psVar13 = (short *)((uint)bVar6 * 5 + iVar7 + 8);
        psVar14[1] = sVar3 + *psVar13;
        sVar5 = psVar13[2];
        *psVar14 = sVar16;
        psVar14[-1] = (char)sVar5 + sVar15;
        psVar14[3] = (ushort)*(byte *)(psVar13 + 1);
        iVar10 = MargePrim(p0,p1);
        if (iVar10 < 0) {
          *p0 = *p0 & 0xff000000 | (uint)p1 & 0xffffff;
          p0 = p1;
        }
        psVar14 = psVar14 + 10;
        p1 = p1 + 5;
        iVar8 = iVar8 + (uint)*(byte *)((int)psVar13 + 3);
        goto switchD_80019650_default;
      }
      switch(bVar6) {
      case 1:
        bVar6 = *pbVar12;
        pbVar12 = param_2 + 4;
        uVar17 = uVar17 & 0xff000000 | (uint)bVar6 | (uint)param_2[2] << 8 |
                 (uint)param_2[3] << 0x10;
        break;
      case 2:
        bVar6 = *pbVar12;
        pbVar12 = param_2 + 2;
        iVar8 = iVar8 + (uint)bVar6;
        break;
      case 3:
        bVar6 = *pbVar12;
        pbVar12 = param_2 + 2;
        sVar16 = sVar16 + (ushort)bVar6;
        break;
      case 4:
        *(uint *)(psVar14 + -3) = uVar17 & 0x3ffffff | 0x40000000;
        *p1 = 0x3000000;
        psVar14[-1] = sVar15;
        bVar6 = *pbVar12;
        pbVar12 = param_2 + 2;
        psVar14[2] = sVar16;
        *psVar14 = sVar16;
        psVar14[1] = sVar15 + (ushort)bVar6;
        iVar10 = MargePrim(p0,p1);
        if (iVar10 < 0) {
          *p0 = *p0 & 0xff000000 | (uint)p1 & 0xffffff;
          p0 = p1;
        }
        psVar14 = psVar14 + 8;
        p1 = p1 + 4;
        break;
      case 5:
        uVar11 = FUN_800191e0(param_1,pbVar12);
        iVar8 = iVar8 - (uVar11 >> 1);
        break;
      case 6:
        iVar10 = FUN_800191e0(param_1,pbVar12);
        iVar8 = iVar8 - iVar10;
        break;
      case 7:
        uVar11 = (uint)*pbVar12;
        pbVar12 = param_2 + 2;
        goto LAB_800197a0;
      case 8:
        bVar6 = *pbVar12;
        pbVar12 = param_2 + 2;
        sVar16 = param_4 + (ushort)bVar6;
        break;
      case 9:
        uVar11 = (iVar8 - param_3) + 0x40U & 0xffffffc0;
LAB_800197a0:
        iVar8 = param_3 + uVar11;
        break;
      case 10:
        sVar16 = sVar16 + (ushort)*(byte *)(*param_1 + 7);
        iVar8 = param_3;
      }
switchD_80019650_default:
      bVar6 = *pbVar12;
      param_2 = pbVar12;
    } while (bVar6 != 0);
  }
  *puVar9 = p0;
  return puVar9;
}

