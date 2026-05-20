// addr: 0x8005030c  name: SetTexWindow

void SetTexWindow(DR_TWIN *p,RECT *tw)

{
  u_long uVar1;
  
  *(undefined1 *)((int)&p->tag + 3) = 2;
  uVar1 = SYS_OBJ_1878(tw);
  p->code[0] = uVar1;
  p->code[1] = 0;
  return;
}

