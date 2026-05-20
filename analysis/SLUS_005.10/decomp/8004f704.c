// addr: 0x8004f704  name: ClearImage

int ClearImage(RECT *rect,u_char r,u_char g,u_char b)

{
  int iVar1;
  
  SYS_OBJ_3E4("ClearImage",rect);
  iVar1 = SYS_OBJ_21F0(SYS_OBJ_19F0,rect,8,(uint)b << 0x10 | (uint)g << 8 | (uint)r);
  return iVar1;
}

