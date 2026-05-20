// addr: 0x8004f8ec  name: MoveImage

int MoveImage(RECT *rect,int x,int y)

{
  int iVar1;
  
  SYS_OBJ_3E4("MoveImage",rect);
  iVar1 = -1;
  if (rect->w != 0) {
    if (rect->h == 0) {
      iVar1 = SYS_OBJ_788();
      return iVar1;
    }
    DAT_800650c8 = y << 0x10 | x & 0xffffU;
    DAT_800650c4._0_2_ = rect->x;
    DAT_800650c4._2_2_ = rect->y;
    DAT_800650cc._0_2_ = rect->w;
    DAT_800650cc._2_2_ = rect->h;
    iVar1 = SYS_OBJ_21F0(SYS_OBJ_2154,&DAT_800650bc,0x14,0);
  }
  return iVar1;
}

