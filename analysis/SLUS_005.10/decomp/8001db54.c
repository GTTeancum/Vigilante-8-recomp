// addr: 0x8001db54  name: FUN_8001db54

bool FUN_8001db54(int *param_1,int param_2)

{
  int iVar1;
  bool bVar2;
  
  gte_SetRotMatrix((MATRIX *)&DAT_8006f780);
  gte_ldsv_(*param_1 - DAT_8006f6f4 >> 8,param_1[1] - DAT_8006f6f8 >> 8,
            param_1[2] - DAT_8006f6fc >> 8);
  gte_rtir();
  param_2 = param_2 >> 8;
  bVar2 = false;
  iVar1 = gte_stIR1();
  if ((iVar1 < param_2) && (iVar1 = gte_stIR2(), iVar1 < param_2)) {
    iVar1 = gte_stIR3();
    bVar2 = iVar1 < param_2;
  }
  return bVar2;
}

