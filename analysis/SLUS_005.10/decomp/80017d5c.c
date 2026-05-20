// addr: 0x80017d5c  name: FUN_80017d5c

void FUN_80017d5c(void)

{
  DRAWENV DStack_68;
  
  SetDefDrawEnv(&DStack_68,0,0,0x400,0x200);
  DStack_68.isbg = '\x01';
  DStack_68.dfe = '\x01';
  PutDrawEnv(&DStack_68);
  VSync(0);
  PutDrawEnv(&DStack_68);
  return;
}

