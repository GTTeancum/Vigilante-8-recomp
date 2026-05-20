// addr: 0x8004d678  name: NormalColorCol3

void NormalColorCol3(SVECTOR *_2,SVECTOR *_3,SVECTOR *v2,CVECTOR *v3,CVECTOR *v4,CVECTOR *v5,
                    CVECTOR *v6)

{
  undefined1 auStackX_0 [16];
  
  gte_ldv3(_2,_3,v2);
  gte_ldrgb(v3);
  gte_ncct_b();
  gte_strgb3(*(CVECTOR **)((int)register0x00000074 + 0x10),
             *(CVECTOR **)((int)register0x00000074 + 0x14),
             *(CVECTOR **)((int)register0x00000074 + 0x18));
  return;
}

