// addr: 0x800522b4  name: SetDrawTPage

void SetDrawTPage(DR_TPAGE *p,int dfe,int dtd,int tpage)

{
  uint uVar1;
  uint uVar2;
  
  *(undefined1 *)((int)&p->tag + 3) = 1;
  uVar2 = 0xe1000000;
  if (dtd != 0) {
    uVar2 = 0xe1000200;
  }
  uVar1 = tpage & 0x9ff;
  if (dfe != 0) {
    uVar1 = uVar1 | 0x400;
  }
  p->code[0] = uVar2 | uVar1;
  return;
}

