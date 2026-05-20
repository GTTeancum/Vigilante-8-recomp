// addr: 0x8004f488  name: DrawSyncCallback

u_long DrawSyncCallback(func *func)

{
  func *pfVar1;
  
  if (1 < DAT_80065026) {
    printf("DrawSyncCallback(%08x)...\n",func);
  }
  pfVar1 = DAT_80065030;
  DAT_80065030 = func;
  return (u_long)pfVar1;
}

