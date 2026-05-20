// addr: 0x800520c4  name: DrawOTag2

int DrawOTag2(u_long *p)

{
  int iVar1;
  
  if (1 < DAT_80065026) {
    printf("DrawOTag(%08x)...\n",p);
  }
  iVar1 = VSync(-1);
  DAT_80065160 = iVar1 + 0xf0;
  DAT_80065164 = 0;
  iVar1 = SYS_OBJ_2F50();
  return iVar1;
}

