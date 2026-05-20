// addr: 0x80020540  name: FUN_80020540

void FUN_80020540(uint *param_1)

{
  if (param_1[0x1a] != 0) {
    FUN_8001bddc();
  }
  if ((*param_1 & 8) != 0) {
    FUN_8003e2c4(param_1[0x1c]);
  }
  FUN_800204dc(param_1);
  return;
}

