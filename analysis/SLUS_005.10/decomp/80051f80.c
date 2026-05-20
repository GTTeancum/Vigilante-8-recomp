// addr: 0x80051f80  name: MoveImage2

int MoveImage2(RECT *rect,int x,int y)

{
  int iVar1;
  
  SYS_OBJ_3E4("MoveImage",rect);
  iVar1 = VSync(-1);
  DAT_80065160 = iVar1 + 0xf0;
  DAT_80065164 = 0;
  iVar1 = SYS_OBJ_2E00();
  return iVar1;
}

