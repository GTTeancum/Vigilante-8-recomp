// addr: 0x8004d6e4  name: ColorCol

void ColorCol(VECTOR *_2,CVECTOR *_3,CVECTOR *v2)

{
  gte_ldlvl(_2);
  gte_ldrgb(_3);
  gte_cc_b();
  gte_strgb(v2);
  return;
}

