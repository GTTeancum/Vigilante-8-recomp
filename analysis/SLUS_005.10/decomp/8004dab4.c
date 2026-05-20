// addr: 0x8004dab4  name: RotMatrixYXZ_gte

MATRIX * RotMatrixYXZ_gte(SVECTOR *r,MATRIX *m)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  undefined4 uVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  undefined4 uVar16;
  int iVar17;
  int iVar18;
  int iVar19;
  
  iVar5._0_2_ = r->vx;
  iVar5._2_2_ = r->vy;
  uVar15 = (int)r->vz >> 0x1f;
  uVar13 = iVar5 >> 0x1f;
  uVar11 = (int)(short)(undefined2)iVar5 >> 0x1f;
  uVar1 = *(int *)(&DAT_800607b4 + (((int)r->vz + uVar15 ^ uVar15) & 0xfff) * 2) * 0x10000 + uVar15
          ^ uVar15;
  uVar2 = *(int *)(&DAT_800607b4 + (((iVar5 >> 0x10) + uVar13 ^ uVar13) & 0xfff) * 2) * 0x10000 +
          uVar13 ^ uVar13;
  uVar3 = *(int *)(&DAT_800607b4 + (((int)(short)(undefined2)iVar5 + uVar11 ^ uVar11) & 0xfff) * 2)
          * 0x10000 + uVar11 ^ uVar11;
  iVar9 = (int)((*(int *)(&DAT_800607b4 + (((iVar5 >> 0x10) + uVar13 ^ uVar13) & 0xfff) * 2) >> 0x10
                ) << 0x10 | uVar2 >> 0x10) >> 0x10;
  gte_ldIR0(iVar9);
  iVar8 = (int)(short)(uVar3 >> 0x10);
  gte_ldIR1(iVar8);
  iVar6 = (int)(short)(uVar1 >> 0x10);
  gte_ldIR2(iVar6);
  iVar4 = (int)((*(int *)(&DAT_800607b4 + (((int)r->vz + uVar15 ^ uVar15) & 0xfff) * 2) >> 0x10) <<
                0x10 | uVar1 >> 0x10) >> 0x10;
  gte_ldIR3(iVar4);
  iVar5 = (int)((*(int *)(&DAT_800607b4 +
                         (((int)(short)(undefined2)iVar5 + uVar11 ^ uVar11) & 0xfff) * 2) >> 0x10)
                << 0x10 | uVar3 >> 0x10) >> 0x10;
  gte_gpf12_b();
  uVar10 = gte_stIR1();
  iVar12 = gte_stIR2();
  iVar14 = gte_stIR3();
  iVar19 = (int)(short)(uVar2 >> 0x10);
  gte_ldIR0(iVar19);
  gte_ldIR1(iVar8);
  gte_ldIR2(iVar6);
  gte_ldIR3(iVar4);
  gte_gpf12_b();
  m->m[2][2] = (short)(iVar5 * iVar9 >> 0xc);
  uVar16 = gte_stIR1();
  iVar17 = gte_stIR2();
  iVar18 = gte_stIR3();
  gte_ldIR0(iVar4);
  gte_ldIR1(iVar5);
  gte_ldIR2(uVar16);
  gte_ldIR3(uVar10);
  gte_gpf12_b();
  uVar1 = gte_stIR1();
  iVar9 = gte_stIR2();
  iVar7 = gte_stIR3();
  gte_ldIR0(iVar6);
  gte_ldsv_(iVar5,uVar16,uVar10);
  gte_gpf12_b();
  *(uint *)(m->m[1] + 1) = uVar1 & 0xffff | iVar8 * -0x10000;
  iVar4 = gte_stIR1();
  iVar6 = gte_stIR2();
  iVar8 = gte_stIR3();
  *(uint *)m->m[0] = (iVar9 - iVar12) * 0x10000 | iVar14 + iVar6 & 0xffffU;
  *(uint *)(m->m[0] + 2) = iVar4 << 0x10 | iVar5 * iVar19 >> 0xc & 0xffffU;
  *(uint *)m->m[2] = (iVar7 + iVar17) * 0x10000 | iVar8 - iVar18 & 0xffffU;
  return m;
}

