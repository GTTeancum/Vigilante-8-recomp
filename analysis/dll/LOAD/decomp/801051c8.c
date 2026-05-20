// addr: 0x801051c8  name: FUN_801051c8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801051c8(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = 0;
  if (0 < _DAT_80065bc4) {
    do {
      iVar2 = *(int *)(iVar3 * 4 + _DAT_80065bd8);
      iVar1 = 0;
      if (0 < *(short *)(iVar2 + 0x12)) {
        do {
          iVar1 = iVar1 + 1;
        } while (iVar1 < *(short *)(iVar2 + 0x12));
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < _DAT_80065bc4);
  }
  iVar3 = 0;
  if (0 < _DAT_80065bc0) {
    do {
      iVar3 = iVar3 + 1;
    } while (iVar3 < _DAT_80065bc0);
  }
  return;
}

