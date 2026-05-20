// addr: 0x80019e7c  name: FUN_80019e7c

void FUN_80019e7c(uint param_1)

{
  int w;
  uint uVar1;
  DISPENV DStack_88;
  DRAWENV DStack_70;
  
  uVar1 = param_1 >> 1 & 1;
  FUN_80018080(param_1 & 1);
  w = 0x280;
  if (uVar1 != 0) {
    w = 0x3c0;
  }
  SetDefDispEnv(&DStack_88,0,0,w,0x1e0);
  SetDefDrawEnv(&DStack_70,0,0,0x280,0x1e0);
  DStack_88.screen.h = 0xf0;
  DStack_70.dfe = '\x01';
  DStack_88.isrgb24 = (u_char)uVar1;
  DStack_70.tpage = 0x20;
  DStack_88.screen.x = (short)cRam00000018;
  DStack_88.screen.y = (short)cRam00000019;
  PutDrawEnv(&DStack_70);
  PutDispEnv(&DStack_88);
  return;
}

