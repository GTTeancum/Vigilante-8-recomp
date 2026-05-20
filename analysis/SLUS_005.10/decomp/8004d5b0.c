// addr: 0x8004d5b0  name: NormalColor3

void NormalColor3(SVECTOR *_2,SVECTOR *_3,SVECTOR *v2,CVECTOR *v3,CVECTOR *v4,CVECTOR *v5)

{
  undefined1 auStackX_0 [16];
  
  gte_ldv3(_2,_3,v2);
  gte_nct_b();
  gte_strgb3(v3,*(CVECTOR **)((int)register0x00000074 + 0x10),
             *(CVECTOR **)((int)register0x00000074 + 0x14));
  return;
}

