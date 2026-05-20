// addr: 0x80020968  name: FUN_80020968

void FUN_80020968(int *param_1)

{
  if (param_1 != (int *)0x0) {
    if (*param_1 == 0) {
      FUN_80020658(param_1 + 1);
    }
    else {
      FUN_80020968(param_1[2]);
      FUN_80020968(param_1[3]);
    }
    FUN_80045088(param_1);
  }
  return;
}

