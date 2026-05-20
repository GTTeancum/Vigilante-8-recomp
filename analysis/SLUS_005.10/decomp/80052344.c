// addr: 0x80052344  name: MargePrim

int MargePrim(void *p0,void *p1)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = (uint)*(byte *)((int)p0 + 3) + (uint)*(byte *)((int)p1 + 3) + 1;
  if (uVar2 < 0x11) {
    *(char *)((int)p0 + 3) = (char)uVar2;
    *(undefined4 *)p1 = 0;
    iVar1 = P36_OBJ_30();
    return iVar1;
  }
  return -1;
}

