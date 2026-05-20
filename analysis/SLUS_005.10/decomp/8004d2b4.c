// addr: 0x8004d2b4  name: ApplyMatrixSV

SVECTOR * ApplyMatrixSV(MATRIX *m,SVECTOR *_2,SVECTOR *_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldR11R12(*(undefined4 *)m->m[0]);
  gte_ldR13R21(*(undefined4 *)(m->m[0] + 2));
  gte_ldR22R23(*(undefined4 *)(m->m[1] + 1));
  gte_ldR31R32(*(undefined4 *)m->m[2]);
  gte_ldR33(*(undefined4 *)(m->m[2] + 2));
  gte_ldv0(_2);
  gte_rtv0_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  _3->vx = (short)uVar1;
  _3->vy = (short)uVar2;
  _3->vz = (short)uVar3;
  return _3;
}

