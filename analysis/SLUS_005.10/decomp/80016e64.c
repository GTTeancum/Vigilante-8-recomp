// addr: 0x80016e64  name: FUN_80016e64

void FUN_80016e64(short *param_1)

{
  long lVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  lVar1 = SquareRoot0((int)*param_1 * (int)*param_1 + (int)param_1[6] * (int)param_1[6]);
  iVar2 = (int)*param_1 * (0x1000000 / lVar1);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  iVar5 = (int)param_1[6] * (0x1000000 / lVar1);
  iVar2 = iVar2 >> 0xc;
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0xfff;
  }
  param_1[8] = (short)lVar1;
  iVar5 = iVar5 >> 0xc;
  iVar3 = iVar2 * param_1[3] - iVar5 * param_1[5];
  param_1[6] = 0;
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  iVar4 = iVar5 * param_1[3] + iVar2 * param_1[5];
  param_1[3] = (short)(iVar3 >> 0xc);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0xfff;
  }
  iVar2 = iVar2 * *param_1 - iVar5 * param_1[2];
  param_1[5] = (short)(iVar4 >> 0xc);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  *param_1 = (short)(iVar2 >> 0xc);
  param_1[2] = 0;
  return;
}

