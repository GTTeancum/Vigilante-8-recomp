// addr: 0x8004d578  name: DpqColor

void DpqColor(CVECTOR *_2,long p,CVECTOR *_3)

{
  gte_ldrgb(_2);
  gte_ldIR0(p);
  gte_dpcs_b();
  gte_strgb(_3);
  return;
}

