// addr: 0x80017324  name: FUN_80017324

void FUN_80017324(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  MATRIX *m;
  
  iVar1 = *(int *)(param_1 + 0x90);
  m = (MATRIX *)(param_1 + 0x10);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  iVar2 = *(int *)(param_1 + 0x94);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  iVar3 = *(int *)(param_1 + 0x98);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  FUN_800439b8(m,iVar1 >> 7,iVar2 >> 7,iVar3 >> 7);
  iVar1 = *(int *)(param_1 + 0x80);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  iVar2 = *(int *)(param_1 + 0x84);
  *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + (iVar1 >> 7);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  iVar1 = *(int *)(param_1 + 0x88);
  *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + (iVar2 >> 7);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + (iVar1 >> 7);
  MatrixNormal(m,m);
  return;
}

