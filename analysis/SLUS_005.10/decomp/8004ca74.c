// addr: 0x8004ca74  name: LoadAverage0

void LoadAverage0(VECTOR *_2,VECTOR *_3,long p0,long p1,VECTOR *v2)

{
  undefined1 auStackX_0 [16];
  
  gte_ldIR0(p0);
  gte_ldlvl(_2);
  gte_gpf0_b(0);
  gte_stLZCR();
  gte_ldIR0(p1);
  gte_ldlvl(_3);
  gte_gpl0_b(0);
  gte_stlvl(*(VECTOR **)((int)register0x00000074 + 0x10));
  return;
}

