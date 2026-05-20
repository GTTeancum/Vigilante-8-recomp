// addr: 0x8004d344  name: SetRotMatrix

void SetRotMatrix(MATRIX *m)

{
  gte_ldR11R12(*(undefined4 *)m->m[0]);
  gte_ldR13R21(*(undefined4 *)(m->m[0] + 2));
  gte_ldR22R23(*(undefined4 *)(m->m[1] + 1));
  gte_ldR31R32(*(undefined4 *)m->m[2]);
  gte_ldR33(*(undefined4 *)(m->m[2] + 2));
  return;
}

