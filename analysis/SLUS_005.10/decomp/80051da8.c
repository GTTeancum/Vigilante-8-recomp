// addr: 0x80051da8  name: LoadImage2

int LoadImage2(RECT *rect,u_long *p)

{
  int iVar1;
  
  SYS_OBJ_3E4("LoadImage2",rect);
  iVar1 = VSync(-1);
  DAT_80065160 = iVar1 + 0xf0;
  DAT_80065164 = 0;
  iVar1 = SYS_OBJ_2C20();
  return iVar1;
}

