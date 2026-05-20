// addr: 0x8004ca24  name: LoadAverage12

void LoadAverage12(VECTOR *_2,VECTOR *_3,long p0,long p1,VECTOR *v2)

{
  undefined1 auStackX_0 [16];
  
  gte_ldIR0(p0);
  gte_ldlvl(_2);
  gte_gpf12_b();
  gte_stLZCR();
  gte_ldIR0(p1);
  gte_ldlvl(_3);
  gte_gpl12_b();
  gte_stlvl(*(VECTOR **)((int)register0x00000074 + 0x10));
  return;
}

