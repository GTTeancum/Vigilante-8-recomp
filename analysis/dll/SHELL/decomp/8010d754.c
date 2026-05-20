// addr: 0x8010d754  name: FUN_8010d754

void FUN_8010d754(undefined4 param_1)

{
  int iVar1;
  undefined1 local_10 [8];
  
  local_10[0] = 0x80;
  do {
    do {
      iVar1 = CdControl/*0x80048fd0*/(2,param_1,0);
    } while (iVar1 == 0);
    do {
      iVar1 = CdControl/*0x80048fd0*/(0xe,local_10,0);
    } while (iVar1 == 0);
    VSync/*0x80047e44*/(3);
    iVar1 = CdRead2/*0x8004b4c4*/(0x1e0);
  } while (iVar1 == 0);
  return;
}

