// addr: 0x80053c90  name: PadGetState

int PadGetState(int param_1)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  
  iVar2 = (*DAT_80065270)();
  if (((*(uint *)(iVar2 + 0x34) & 0xffff0000) == 0) &&
     (((iVar2 == *(int *)(iVar2 + 0x10) || (*(char *)(iVar2 + 0x38) == '\0')) &&
      (**(char **)(iVar2 + 0x30) == '\0')))) {
    uVar3 = (uint)*(byte *)(iVar2 + 0x49);
  }
  else {
    bVar1 = *(byte *)(iVar2 + 0x49);
    if (bVar1 == 3) {
      iVar2 = PADENTRY_OBJ_15C();
      return iVar2;
    }
    if (bVar1 < 4) {
      uVar3 = 1;
      if (bVar1 != 2) {
        iVar2 = PADENTRY_OBJ_158();
        return iVar2;
      }
    }
    else {
      uVar3 = 4;
      if (bVar1 != 6) {
        iVar2 = PADENTRY_OBJ_158();
        return iVar2;
      }
    }
  }
  return uVar3;
}

