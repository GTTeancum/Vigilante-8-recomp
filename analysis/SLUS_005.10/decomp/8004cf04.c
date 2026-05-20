// addr: 0x8004cf04  name: CompMatrixLV

MATRIX * CompMatrixLV(MATRIX *m0,MATRIX *m1,MATRIX *m2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  
  gte_ldR11R12(*(undefined4 *)m0->m[0]);
  gte_ldR13R21(*(undefined4 *)(m0->m[0] + 2));
  gte_ldR22R23(*(undefined4 *)(m0->m[1] + 1));
  gte_ldR31R32(*(undefined4 *)m0->m[2]);
  gte_ldR33(*(undefined4 *)(m0->m[2] + 2));
  gte_ldVXY0((uint)(ushort)m1->m[0][0] | *(uint *)(m1->m[0] + 2) & 0xffff0000);
  gte_ldVZ0(*(undefined4 *)m1->m[2]);
  gte_rtv0_b();
  uVar1 = gte_stIR1();
  iVar2 = gte_stIR2();
  uVar4 = gte_stIR3();
  gte_ldVXY0((uint)(ushort)m1->m[0][1] | *(int *)(m1->m[1] + 1) << 0x10);
  gte_ldVZ0((int)m1->m[2][1]);
  gte_rtv0_b();
  iVar6 = gte_stIR1();
  uVar7 = gte_stIR2();
  iVar8 = gte_stIR3();
  gte_ldVXY0((uint)(ushort)m1->m[0][2] | *(uint *)(m1->m[1] + 1) & 0xffff0000);
  gte_ldVZ0(*(undefined4 *)(m1->m[2] + 2));
  gte_rtv0_b();
  *(uint *)m2->m[0] = iVar6 << 0x10 | uVar1 & 0xffff;
  *(uint *)m2->m[2] = iVar8 << 0x10 | uVar4 & 0xffff;
  uVar1 = gte_stIR1();
  iVar6 = gte_stIR2();
  gte_stIR3();
  *(uint *)(m2->m[0] + 2) = uVar1 & 0xffff | iVar2 << 0x10;
  *(uint *)(m2->m[1] + 1) = iVar6 << 0x10 | uVar7 & 0xffff;
  uVar1 = m1->t[0];
  uVar4 = m1->t[1];
  uVar7 = m1->t[2];
  if ((int)uVar1 < 0) {
    iVar6 = -((int)-uVar1 >> 0xf);
    uVar1 = -(-uVar1 & 0x7fff);
  }
  else {
    iVar6 = (int)uVar1 >> 0xf;
    uVar1 = uVar1 & 0x7fff;
  }
  if ((int)uVar4 < 0) {
    iVar2 = -((int)-uVar4 >> 0xf);
    uVar4 = -(-uVar4 & 0x7fff);
  }
  else {
    iVar2 = (int)uVar4 >> 0xf;
    uVar4 = uVar4 & 0x7fff;
  }
  if ((int)uVar7 < 0) {
    iVar8 = -((int)-uVar7 >> 0xf);
    uVar7 = -(-uVar7 & 0x7fff);
  }
  else {
    iVar8 = (int)uVar7 >> 0xf;
    uVar7 = uVar7 & 0x7fff;
  }
  gte_ldsv_(iVar6,iVar2,iVar8);
  gte_rtir_sf0_b();
  read_mt(iVar6,iVar2,iVar8);
  gte_ldsv_(uVar1,uVar4,uVar7);
  gte_rtir_b();
  if (iVar6 < 0) {
    iVar6 = iVar6 * 8;
  }
  else {
    iVar6 = iVar6 << 3;
  }
  if (iVar2 < 0) {
    iVar2 = iVar2 * 8;
  }
  else {
    iVar2 = iVar2 << 3;
  }
  if (iVar8 < 0) {
    iVar8 = iVar8 * 8;
  }
  else {
    iVar8 = iVar8 << 3;
  }
  read_mt(uVar1,uVar4,uVar7);
  iVar3 = m0->t[1];
  iVar5 = m0->t[2];
  m2->t[0] = uVar1 + iVar6 + m0->t[0];
  m2->t[1] = uVar4 + iVar2 + iVar3;
  m2->t[2] = uVar7 + iVar8 + iVar5;
  return m2;
}

