// addr: 0x8002cbe8  name: FUN_8002cbe8

void FUN_8002cbe8(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  
  *(undefined2 *)(param_2 + 6) = 0;
  FUN_8001d4f0();
  uVar3 = 0;
  if (*(int *)(param_1 + 0x110) != 0) {
    iVar2 = 0x24;
    iVar1 = param_1;
    do {
      if (2 < (int)uVar3) break;
      iVar2 = iVar2 + 4;
      if (*(char *)(*(int *)(iVar1 + 0x110) + 8) == *(char *)(param_2 + 8)) {
        *(char *)(param_2 + 8) = -*(char *)(param_2 + 8);
        return;
      }
      iVar1 = iVar1 + 4;
      uVar3 = uVar3 + 1;
    } while (*(int *)(param_1 + iVar2 + 0xec) != 0);
  }
  if (uVar3 == 3) {
    uVar4 = (uint)*(byte *)(param_1 + 0xb3);
    uVar3 = uVar4;
    if (((0 < *(short *)(param_1 + 6)) &&
        (*(char *)(*(int *)(param_1 + uVar4 * 4 + 0x110) + 8) == '\x06')) && (uVar3 = 2, uVar4 != 0)
       ) {
      uVar3 = uVar4 - 1;
    }
    FUN_8002c99c(param_1,uVar3);
  }
  *(int *)(param_1 + (uVar3 + 9) * 4 + 0xec) = param_2;
  return;
}

