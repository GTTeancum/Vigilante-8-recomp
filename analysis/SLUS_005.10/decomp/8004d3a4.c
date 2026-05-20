// addr: 0x8004d3a4  name: SetColorMatrix

void SetColorMatrix(MATRIX *m)

{
  gte_ldLR1LR2(*(undefined4 *)m->m[0]);
  gte_ldLR3LG1(*(undefined4 *)(m->m[0] + 2));
  gte_ldLG2LG3(*(undefined4 *)(m->m[1] + 1));
  gte_ldLB1LB2(*(undefined4 *)m->m[2]);
  gte_ldLB3(*(undefined4 *)(m->m[2] + 2));
  return;
}

