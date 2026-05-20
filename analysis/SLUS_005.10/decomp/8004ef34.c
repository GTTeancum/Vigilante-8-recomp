// addr: 0x8004ef34  name: LoadTPage

u_short LoadTPage(u_long *pix,int tp,int abr,int x,int y,int w,int h)

{
  u_short uVar1;
  RECT local_20;
  
  local_20.x = (short)x;
  local_20.h = (short)h;
  local_20.y = (short)y;
  if (tp == 1) {
    local_20.w = (short)(w / 2);
    uVar1 = EXT_OBJ_A8();
    return uVar1;
  }
  if (tp < 2) {
    if (tp != 0) {
      uVar1 = EXT_OBJ_AC();
      return uVar1;
    }
    if (w < 0) {
      w = w + 3;
    }
    local_20.w = (short)(w >> 2);
    uVar1 = EXT_OBJ_A8(&local_20);
    return uVar1;
  }
  if (tp != 2) {
    uVar1 = EXT_OBJ_AC(&local_20);
    return uVar1;
  }
  local_20.w = (short)w;
  LoadImage(&local_20,pix);
  uVar1 = GetTPage(2,abr,x,y);
  return uVar1;
}

