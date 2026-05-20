// addr: 0x8004d610  name: NormalColorDpq3

void NormalColorDpq3(SVECTOR *_2,SVECTOR *_3,SVECTOR *v2,CVECTOR *v3,long p,CVECTOR *v4,CVECTOR *v5,
                    CVECTOR *v6)

{
  undefined1 auStackX_0 [16];
  
  gte_ldv3(_2,_3,v2);
  gte_ldrgb(v3);
  gte_ldIR0((undefined1 *)((int)register0x00000074 + 0x10));
  gte_ncdt_b();
  gte_strgb3(*(CVECTOR **)((int)register0x00000074 + 0x14),
             *(CVECTOR **)((int)register0x00000074 + 0x18),
             *(CVECTOR **)((int)register0x00000074 + 0x1c));
  return;
}

