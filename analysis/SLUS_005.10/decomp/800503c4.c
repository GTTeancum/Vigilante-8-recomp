// addr: 0x800503c4  name: SetDrawOffset

void SetDrawOffset(DR_OFFSET *p,u_short *ofs)

{
  u_long uVar1;
  
  *(undefined1 *)((int)&p->tag + 3) = 2;
  uVar1 = SYS_OBJ_185C((int)(short)*ofs,(int)(short)ofs[1]);
  p->code[0] = uVar1;
  p->code[1] = 0;
  return;
}

