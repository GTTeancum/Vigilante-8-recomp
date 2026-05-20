// addr: 0x8004d734  name: TransposeMatrix

MATRIX * TransposeMatrix(MATRIX *m0,MATRIX *m1)

{
  short sVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  uVar2 = *(undefined4 *)m0->m[0];
  uVar3 = *(undefined4 *)(m0->m[0] + 2);
  *(undefined4 *)(m1->m[0] + 2) = uVar2;
  *(undefined4 *)m1->m[0] = uVar3;
  m1->m[0][0] = (short)uVar2;
  uVar4 = *(undefined4 *)(m0->m[1] + 1);
  uVar2 = *(undefined4 *)m0->m[2];
  *(undefined4 *)m1->m[2] = uVar4;
  *(undefined4 *)(m1->m[1] + 1) = uVar2;
  m1->m[2][0] = (short)uVar3;
  m1->m[1][1] = (short)uVar4;
  sVar1 = m0->m[2][2];
  m1->m[0][2] = (short)uVar2;
  m1->m[2][2] = sVar1;
  return m1;
}

