// addr: 0x8004d6bc  name: ColorDpq

void ColorDpq(VECTOR *_2,CVECTOR *_3,long p,CVECTOR *v2)

{
  gte_ldlvl(_2);
  gte_ldrgb(_3);
  gte_ldIR0(p);
  gte_cdp_b();
  gte_strgb(v2);
  return;
}

