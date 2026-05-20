// addr: 0x80019cbc  name: FUN_80019cbc

void FUN_80019cbc(undefined2 *param_1,char *param_2,undefined2 param_3,undefined2 param_4)

{
  strcpy((char *)(param_1 + 4),param_2);
  param_1[1] = 0;
  *param_1 = 0x200;
  param_1[2] = param_3;
  param_1[3] = param_4;
  return;
}

