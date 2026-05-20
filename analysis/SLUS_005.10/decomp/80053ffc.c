// addr: 0x80053ffc  name: PadSetMainMode

int PadSetMainMode(int socket,int offs,int lock)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = (*DAT_80065270)();
  iVar2 = _padSetMainMode(uVar1,offs & 0xff,lock & 0xff);
  return iVar2;
}

