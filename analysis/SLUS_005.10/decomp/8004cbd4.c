// addr: 0x8004cbd4  name: LoadAverageByte

void LoadAverageByte(u_char *_2,u_char *_3,long p0,long p1,u_char *v2)

{
  int iVar1;
  int iVar2;
  
  gte_ldIR0(p0);
  gte_ldIR1((uint)*_2);
  gte_ldIR2((uint)_2[1]);
  gte_gpf0_b(0);
  gte_stLZCR();
  gte_ldIR0(p1);
  gte_ldIR1((uint)*_3);
  gte_ldIR2((uint)_3[1]);
  gte_gpl0_b(0);
  iVar1 = gte_stMAC1();
  iVar2 = gte_stMAC2();
  *v2 = (u_char)(iVar1 >> 0xc);
  v2[1] = (u_char)(iVar2 >> 0xc);
  return;
}

