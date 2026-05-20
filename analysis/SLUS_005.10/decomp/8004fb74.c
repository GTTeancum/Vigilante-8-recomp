// addr: 0x8004fb74  name: DrawOTag

void DrawOTag(u_long *p)

{
  if (1 < DAT_80065026) {
    printf("DrawOTag(%08x)...\n",p);
  }
  SYS_OBJ_21F0(SYS_OBJ_2154,p,0,0);
  return;
}

