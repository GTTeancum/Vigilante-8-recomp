// addr: 0x8004f9a4  name: ClearOTag

u_long * ClearOTag(u_long *ot,int n)

{
  if (1 < DAT_80065026) {
    printf("ClearOTag(%08x,%d)...\n",ot,n);
  }
  while (n = n + -1, n != 0) {
    *(undefined1 *)((int)ot + 3) = 0;
    *ot = *ot & 0xff000000 | (uint)(ot + 1) & 0xffffff;
    ot = ot + 1;
  }
  DAT_800650e4 = 0x40650d0;
  *ot = 0x650e4;
  return ot;
}

