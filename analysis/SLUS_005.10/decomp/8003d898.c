// addr: 0x8003d898  name: FUN_8003d898

void FUN_8003d898(int param_1)

{
  int iVar1;
  undefined2 uVar2;
  
  iVar1 = iRam00000010;
  uVar2 = 0xff00;
  if (iRam00000010 == 1) {
    uVar2 = 0xff60;
  }
  *(undefined2 *)(param_1 + 0x8c) = uVar2;
  uVar2 = 0xa0;
  if (iVar1 == 1) {
    uVar2 = 0x50;
  }
  *(undefined2 *)(param_1 + 0x90) = uVar2;
  return;
}

