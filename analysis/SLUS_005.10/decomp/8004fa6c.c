// addr: 0x8004fa6c  name: ClearOTagR

u_long * ClearOTagR(u_long *ot,int n)

{
  if (1 < DAT_80065026) {
    printf("ClearOTagR(%08x,%d)...\n",ot,n);
  }
  SYS_OBJ_1910(ot,n);
  DAT_800650e4 = 0x40650d0;
  *ot = 0x650e4;
  return ot;
}

