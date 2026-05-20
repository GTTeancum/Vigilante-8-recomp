// addr: 0x800173fc  name: FUN_800173fc

void FUN_800173fc(int param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  MATRIX *m;
  
  *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) + *param_2;
  *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) + param_2[1];
  *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + param_2[2];
  iVar1 = *param_3 * (int)*(short *)(param_1 + 0x9c);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) + (iVar1 >> 6);
  iVar1 = param_3[1] * (int)*(short *)(param_1 + 0x9e);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  *(int *)(param_1 + 0x94) = *(int *)(param_1 + 0x94) + (iVar1 >> 6);
  iVar1 = param_3[2] * (int)*(short *)(param_1 + 0xa0);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  iVar2 = *(int *)(param_1 + 0x90);
  m = (MATRIX *)(param_1 + 0x10);
  iVar1 = *(int *)(param_1 + 0x98) + (iVar1 >> 6);
  *(int *)(param_1 + 0x98) = iVar1;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  iVar3 = *(int *)(param_1 + 0x94);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  FUN_800439b8(m,iVar2 >> 7,iVar3 >> 7,iVar1 >> 7);
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

