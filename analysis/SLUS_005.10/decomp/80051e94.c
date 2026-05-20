// addr: 0x80051e94  name: StoreImage2

int StoreImage2(RECT *rect,u_long *p)

{
  int iVar1;
  
  SYS_OBJ_3E4("StoreImage",rect);
  iVar1 = VSync(-1);
  DAT_80065160 = iVar1 + 0xf0;
  DAT_80065164 = 0;
  iVar1 = SYS_OBJ_2D0C();
  return iVar1;
}

