// addr: 0x8001b07c  name: FUN_8001b07c

MATRIX * FUN_8001b07c(MATRIX *param_1,int param_2)

{
  RotMatrixYXZ_gte((SVECTOR *)(param_2 + 0x10),param_1);
  FUN_8004d314(param_1,param_2 + 4);
  return param_1;
}

