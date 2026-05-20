// addr: 0x80053e48  name: PadInfoAct

int PadInfoAct(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  iVar1 = (*DAT_80065270)();
  if (-1 < param_2) {
    if (param_2 < (int)(uint)*(byte *)(iVar1 + 0xe9)) {
      switch(param_3) {
      case 1:
        iVar1 = PADENTRY_OBJ_324();
        return iVar1;
      case 2:
        iVar1 = PADENTRY_OBJ_324();
        return iVar1;
      case 3:
        iVar1 = PADENTRY_OBJ_324();
        return iVar1;
      case 4:
        iVar1 = PADENTRY_OBJ_324();
        return iVar1;
      case 5:
        iVar1 = PADENTRY_OBJ_324();
        return iVar1;
      }
    }
    return 0;
  }
  iVar1 = PADENTRY_OBJ_324();
  return iVar1;
}

