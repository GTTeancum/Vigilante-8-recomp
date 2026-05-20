// addr: 0x80046044  name: SpuInitMalloc

long SpuInitMalloc(long num,char *top)

{
  uint uVar1;
  long lVar2;
  
  uVar1 = DAT_8005edfc;
  if (num < 1) {
    lVar2 = S_M_INIT_OBJ_4C();
    return lVar2;
  }
  top[0] = '\x10';
  top[1] = '\x10';
  top[2] = '\0';
  top[3] = '@';
  DAT_8005ee38 = 0;
  DAT_8005ee34 = num;
  DAT_8005ee3c = top;
  *(int *)(top + 4) = (0x10000 << (uVar1 & 0x1f)) + -0x1010;
  return num;
}

