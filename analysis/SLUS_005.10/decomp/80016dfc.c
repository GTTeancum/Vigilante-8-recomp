// addr: 0x80016dfc  name: FUN_80016dfc

void FUN_80016dfc(MATRIX *param_1,MATRIX *param_2)

{
  int iVar1;
  int iVar2;
  
  TransposeMatrix(param_1,param_2);
  FUN_80043358(param_2,param_1->t,param_2->t);
  iVar1 = param_2->t[1];
  iVar2 = param_2->t[2];
  param_2->t[0] = -param_2->t[0];
  param_2->t[1] = -iVar1;
  param_2->t[2] = -iVar2;
  return;
}

