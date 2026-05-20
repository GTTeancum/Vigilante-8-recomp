// addr: 0x8001d68c  name: FUN_8001d68c

void FUN_8001d68c(MATRIX *param_1,undefined4 param_2,undefined4 param_3)

{
  MATRIX *m0;
  MATRIX *m1;
  
  m0 = (MATRIX *)FUN_8001d624(param_2);
  m1 = (MATRIX *)FUN_8001b07c(param_1,param_3);
  CompMatrixLV(m0,m1,param_1);
  return;
}

