// addr: 0x8004f580  name: DrawSync

int DrawSync(int mode)

{
  int iVar1;
  
  if (1 < DAT_80065026) {
    printf("DrawSync(%d)...\n",mode);
  }
  iVar1 = SYS_OBJ_2850(mode);
  return iVar1;
}

