// addr: 0x80019e20  name: FUN_80019e20

void FUN_80019e20(void)

{
  DRAWENV DStack_68;
  
  SetDefDrawEnv(&DStack_68,0,0,0x280,0x1e0);
  DStack_68.dfe = '\x01';
  DStack_68.tpage = GetTPage(0,1,0,0);
  PutDrawEnv(&DStack_68);
  return;
}

