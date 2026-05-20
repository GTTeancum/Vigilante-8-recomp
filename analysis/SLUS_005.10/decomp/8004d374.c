// addr: 0x8004d374  name: SetLightMatrix

void SetLightMatrix(MATRIX *m)

{
  gte_ldL11L12(*(undefined4 *)m->m[0]);
  gte_ldL13L21(*(undefined4 *)(m->m[0] + 2));
  gte_ldL22L23(*(undefined4 *)(m->m[1] + 1));
  gte_ldL31L32(*(undefined4 *)m->m[2]);
  gte_ldL33(*(undefined4 *)(m->m[2] + 2));
  return;
}

