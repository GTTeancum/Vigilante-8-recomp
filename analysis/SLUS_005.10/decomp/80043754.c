// addr: 0x80043754  name: FUN_80043754

int * FUN_80043754(int *param_1,int *param_2,uint *param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar3 = *param_2;
  iVar4 = param_2[1];
  iVar5 = param_2[2];
  iVar1 = param_1[1];
  *param_3 = (short)*param_1 * iVar3 >> 0xc & 0xffffU | ((*param_1 >> 0x10) * iVar4 >> 0xc) << 0x10;
  iVar2 = param_1[2];
  param_3[1] = (short)iVar1 * iVar5 >> 0xc & 0xffffU | ((iVar1 >> 0x10) * iVar3 >> 0xc) << 0x10;
  iVar1 = param_1[3];
  param_3[2] = (short)iVar2 * iVar4 >> 0xc & 0xffffU | ((iVar2 >> 0x10) * iVar5 >> 0xc) << 0x10;
  iVar2 = param_1[4];
  param_3[3] = (short)iVar1 * iVar3 >> 0xc & 0xffffU | ((iVar1 >> 0x10) * iVar4 >> 0xc) << 0x10;
  param_3[4] = (short)iVar2 * iVar5 >> 0xc;
  return param_1;
}

