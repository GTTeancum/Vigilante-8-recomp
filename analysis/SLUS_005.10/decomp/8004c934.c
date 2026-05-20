// addr: 0x8004c934  name: MatrixNormal

void MatrixNormal(MATRIX *m,MATRIX *n)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  short sVar4;
  short sVar5;
  int iVar6;
  short sVar7;
  short sVar8;
  int iVar9;
  short sVar10;
  short sVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  undefined4 in_t7;
  undefined4 in_t8;
  undefined4 in_t9;
  
  iVar6 = (int)m->m[0][0];
  iVar9 = (int)m->m[0][1];
  iVar12 = (int)m->m[0][2];
  iVar13 = (int)m->m[1][0];
  iVar14 = (int)m->m[1][1];
  iVar15 = (int)m->m[1][2];
  uVar1 = gte_stR11R12();
  uVar2 = gte_stR22R23();
  uVar3 = gte_stR33();
  gte_ldR11R12(iVar6);
  gte_ldR22R23(iVar9);
  gte_ldR33(iVar12);
  gte_ldIR3(iVar15);
  gte_ldIR1(iVar13);
  gte_ldIR2(iVar14);
  gte_op12_b();
  read_mt(in_t7,in_t8,in_t9);
  gte_ldR11R12(iVar13);
  gte_ldR22R23(iVar14);
  gte_ldR33(iVar15);
  gte_op12_b();
  gte_ldVXY0(iVar13);
  gte_ldVZ0(iVar14);
  gte_ldVXY1(iVar15);
  read_mt(iVar6,iVar9,iVar12);
  gte_ldR11R12(uVar1);
  gte_ldR22R23(uVar2);
  gte_ldR33(uVar3);
  MSC02_OBJ_100(m,n,uVar3);
  sVar11 = (short)in_t9;
  sVar8 = (short)in_t8;
  sVar5 = (short)in_t7;
  n->m[0][0] = (short)iVar6;
  n->m[0][1] = (short)iVar9;
  n->m[0][2] = (short)iVar12;
  uVar1 = gte_stVXY0();
  sVar4 = (short)uVar1;
  uVar1 = gte_stVZ0();
  sVar7 = (short)uVar1;
  uVar1 = gte_stVXY1();
  sVar10 = (short)uVar1;
  MSC02_OBJ_100();
  n->m[1][0] = sVar4;
  n->m[1][1] = sVar7;
  n->m[1][2] = sVar10;
  MSC02_OBJ_100();
  n->m[2][0] = sVar5;
  n->m[2][1] = sVar8;
  n->m[2][2] = sVar11;
  return;
}

