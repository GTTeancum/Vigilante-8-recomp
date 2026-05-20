// addr: 0x8004cc34  name: LoadAverageCol

void LoadAverageCol(u_char *_2,u_char *_3,long p0,long p1,u_char *v2)

{
  gte_ldIR0(p0);
  gte_ldsv_((uint)*_2,(uint)_2[1],(uint)_2[2]);
  gte_gpf0_b(0);
  gte_stLZCR();
  gte_ldIR0(p1);
  gte_ldsv_((uint)*_3,(uint)_3[1],(uint)_3[2]);
  gte_gpl0_b(0);
  read_mt((uint)*_3,(uint)_3[1],(uint)_3[2]);
  *v2 = '\0';
  v2[1] = '\0';
  v2[2] = '\0';
  return;
}

