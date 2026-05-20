// addr: 0x800522e4  name: SetDrawMove

void SetDrawMove(DR_MOVE *p,RECT *rect,int x,int y)

{
  undefined1 uVar1;
  u_long uVar2;
  
  uVar1 = 5;
  if ((rect->w == 0) || (rect->h == 0)) {
    uVar1 = 0;
  }
  p->code[0] = 0x1000000;
  p->code[1] = 0x80000000;
  *(undefined1 *)((int)&p->tag + 3) = uVar1;
  uVar2 = *(u_long *)rect;
  p->code[3] = y << 0x10 | x & 0xffffU;
  p->code[2] = uVar2;
  p->code[4] = *(u_long *)&rect->w;
  return;
}

