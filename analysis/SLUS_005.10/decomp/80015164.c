// addr: 0x80015164  name: FUN_80015164

void FUN_80015164(int param_1)

{
  undefined4 uVar1;
  DISPENV DStack_88;
  DRAWENV DStack_70;
  undefined2 local_10;
  undefined2 local_e;
  short local_c;
  short local_a;
  
  local_c = 0x140;
  local_10 = 0;
  local_e = 0;
  local_a = 0xf0;
  FUN_80017fd4(0);
  if (param_1 != 0) {
    local_c = 0x280;
    local_a = 0x1e0;
  }
  uVar1 = FUN_80015948("Misc\\Kong.fnt");
  uRam00000684 = FUN_80019034(uVar1,0);
  SetDefDispEnv(&DStack_88,0,0,(int)local_c,(int)local_a);
  SetDefDrawEnv(&DStack_70,0,0,(int)local_c,(int)local_a);
  DStack_70.dfe = '\x01';
  DStack_70.tpage = 0x20;
  PutDrawEnv(&DStack_70);
  PutDispEnv(&DStack_88);
  FUN_8001a0ac(&local_10,0);
  iRam00000694 = local_a + -0x20;
  SetDispMask(1);
  uRam0000068c = 0x20;
  uRam00000688 = 0x20;
  uRam00000690 = 0;
  return;
}

