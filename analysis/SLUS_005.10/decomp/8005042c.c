// addr: 0x8005042c  name: SetDrawStp

void SetDrawStp(DR_STP *p,int pbw)

{
  *(undefined1 *)((int)&p->tag + 3) = 2;
  if (pbw != 0) {
    SYS_OBJ_1244();
    return;
  }
  p->code[0] = 0xe6000000;
  p->code[1] = 0;
  return;
}

