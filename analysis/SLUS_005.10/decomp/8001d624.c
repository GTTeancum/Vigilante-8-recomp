// addr: 0x8001d624  name: FUN_8001d624

MATRIX * FUN_8001d624(int param_1)

{
  MATRIX *m1;
  
  m1 = (MATRIX *)(param_1 + 0x10);
  while( true ) {
    param_1 = FUN_8001d5a0(param_1);
    if (param_1 == 0) break;
    CompMatrixLV((MATRIX *)(param_1 + 0x10),m1,(MATRIX *)&DAT_8006f640);
    m1 = (MATRIX *)&DAT_8006f640;
  }
  return m1;
}

