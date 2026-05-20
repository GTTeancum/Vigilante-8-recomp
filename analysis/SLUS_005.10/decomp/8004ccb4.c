// addr: 0x8004ccb4  name: MulMatrix0

MATRIX * MulMatrix0(MATRIX *m0,MATRIX *m1,MATRIX *m2)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  
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
  uVar3 = gte_stIR3();
  gte_ldVXY0((uint)(ushort)m1->m[0][1] | *(int *)(m1->m[1] + 1) << 0x10);
  gte_ldVZ0((int)m1->m[2][1]);
  gte_rtv0_b();
  iVar4 = gte_stIR1();
  uVar5 = gte_stIR2();
  iVar6 = gte_stIR3();
  gte_ldVXY0((uint)(ushort)m1->m[0][2] | *(uint *)(m1->m[1] + 1) & 0xffff0000);
  gte_ldVZ0(*(undefined4 *)(m1->m[2] + 2));
  gte_rtv0_b();
  *(uint *)m2->m[0] = iVar4 << 0x10 | uVar1 & 0xffff;
  *(uint *)m2->m[2] = iVar6 << 0x10 | uVar3 & 0xffff;
  uVar1 = gte_stIR1();
  iVar4 = gte_stIR2();
  *(uint *)(m2->m[0] + 2) = uVar1 & 0xffff | iVar2 << 0x10;
  *(uint *)(m2->m[1] + 1) = iVar4 << 0x10 | uVar5 & 0xffff;
  gte_stIR3();
  return m2;
}

