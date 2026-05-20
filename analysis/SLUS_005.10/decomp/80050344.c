// addr: 0x80050344  name: SetDrawArea

void SetDrawArea(DR_AREA *p,RECT *r)

{
  u_long uVar1;
  
  *(undefined1 *)((int)&p->tag + 3) = 2;
  uVar1 = SYS_OBJ_172C((int)r->x,(int)r->y);
  p->code[0] = uVar1;
  uVar1 = SYS_OBJ_17C4((int)(((uint)(ushort)r->x + (uint)(ushort)r->w + -1) * 0x10000) >> 0x10,
                       (int)(((uint)(ushort)r->y + (uint)(ushort)r->h + -1) * 0x10000) >> 0x10);
  p->code[1] = uVar1;
  return;
}

