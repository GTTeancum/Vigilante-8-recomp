// addr: 0x8001fc38  name: FUN_8001fc38

void FUN_8001fc38(uint *param_1,undefined2 param_2)

{
  int iVar1;
  uint *puVar2;
  
  do {
    puVar2 = (uint *)param_1[0xd];
    if ((((*param_1 & 4) == 0) && (iVar1 = FUN_8001f9cc(param_1,param_2), -1 < iVar1)) &&
       (param_1[0xe] != 0)) {
      FUN_8001fc38(param_1[0xe],param_2);
    }
    param_1 = puVar2;
  } while (puVar2 != (uint *)0x0);
  return;
}

