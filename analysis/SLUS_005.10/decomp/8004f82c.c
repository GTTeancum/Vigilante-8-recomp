// addr: 0x8004f82c  name: LoadImage

int LoadImage(RECT *rect,u_long *p)

{
  int iVar1;
  
  SYS_OBJ_3E4("LoadImage",rect);
  iVar1 = SYS_OBJ_21F0(SYS_OBJ_1C20,rect,8,p);
  return iVar1;
}

