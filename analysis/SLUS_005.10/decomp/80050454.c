// addr: 0x80050454  name: SetDrawMode

void SetDrawMode(DR_MODE *p,int dfe,int dtd,int tpage,RECT *tw)

{
  u_long uVar1;
  
  *(undefined1 *)((int)&p->tag + 3) = 2;
  uVar1 = SYS_OBJ_170C(dfe,dtd,tpage & 0xffff);
  p->code[0] = uVar1;
  uVar1 = SYS_OBJ_1878(tw);
  p->code[1] = uVar1;
  return;
}

