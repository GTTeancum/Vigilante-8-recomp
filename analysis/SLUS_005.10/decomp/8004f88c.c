// addr: 0x8004f88c  name: StoreImage

int StoreImage(RECT *rect,u_long *p)

{
  int iVar1;
  
  SYS_OBJ_3E4("StoreImage",rect);
  iVar1 = SYS_OBJ_21F0(SYS_OBJ_1E5C,rect,8,p);
  return iVar1;
}

