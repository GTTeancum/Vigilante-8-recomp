// addr: 0x8002c99c  name: FUN_8002c99c

void FUN_8002c99c(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar1 = FUN_8003ff28(*(undefined4 *)(param_1 + (param_2 + 9) * 4 + 0xec));
  *(undefined2 *)(iVar1 + 6) = *(undefined2 *)(param_1 + 6);
  iVar3 = *(int *)(param_1 + 0x80);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  *(int *)(iVar1 + 0x88) = *(int *)(iVar1 + 0x88) + (iVar3 >> 7);
  iVar3 = *(int *)(param_1 + 0x84);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  *(int *)(iVar1 + 0x8c) = *(int *)(iVar1 + 0x8c) + (iVar3 >> 7);
  iVar3 = *(int *)(param_1 + 0x88);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  *(int *)(iVar1 + 0x90) = *(int *)(iVar1 + 0x90) + (iVar3 >> 7);
  uVar2 = FUN_8004410c();
  FUN_8004483c(uVar2,uRam000005f8,0x29,iVar1 + 0x24);
  if ((*(short *)(iVar1 + 0xc) != 0) && (*(char *)(iVar1 + 8) != '\x06')) {
    *(undefined1 **)(iVar1 + 100) = &LAB_8003cb64;
  }
  return;
}

