// addr: 0x8001d9c0  name: FUN_8001d9c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8001d9c0(MATRIX *param_1,undefined4 param_2)

{
  DAT_8006f6e0 = *(undefined4 *)param_1->m[0];
  DAT_8006f6e4 = *(undefined4 *)(param_1->m[0] + 2);
  DAT_8006f6e8 = *(undefined4 *)(param_1->m[1] + 1);
  DAT_8006f6ec = *(undefined4 *)param_1->m[2];
  DAT_8006f6f0 = *(undefined4 *)(param_1->m[2] + 2);
  DAT_8006f6f4 = param_1->t[0];
  DAT_8006f6f8 = param_1->t[1];
  DAT_8006f6fc = param_1->t[2];
  DAT_8006f740 = DAT_8006f6e0;
  DAT_8006f744 = DAT_8006f6e4;
  DAT_8006f748 = DAT_8006f6e8;
  DAT_8006f74c = DAT_8006f6ec;
  DAT_8006f750 = DAT_8006f6f0;
  DAT_8006f754 = DAT_8006f6f4;
  DAT_8006f758 = DAT_8006f6f8;
  DAT_8006f75c = DAT_8006f6fc;
  FUN_80016dfc(param_1,&DAT_8006f680);
  uRam000006d4 = param_2;
  FUN_8004d544();
  MulMatrix0((MATRIX *)&DAT_8006f720,param_1,(MATRIX *)&DAT_8006f700);
  FUN_8001d898(&DAT_8006f680);
  DAT_8006f660 = DAT_8006f680;
  DAT_8006f664 = DAT_8006f684;
  DAT_8006f668 = DAT_8006f688;
  DAT_8006f66c = DAT_8006f68c;
  _DAT_8006f670 = DAT_8006f690;
  DAT_8006f674 = DAT_8006f694;
  DAT_8006f678 = DAT_8006f698;
  DAT_8006f67c = DAT_8006f69c;
  FUN_80016e64(&DAT_8006f660);
  return;
}

