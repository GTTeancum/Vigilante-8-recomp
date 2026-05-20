// addr: 0x8001d708  name: FUN_8001d708

void FUN_8001d708(int param_1)

{
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x48);
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_1 + 0x4c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x50);
  RotMatrixYXZ_gte((SVECTOR *)(param_1 + 0x40),(MATRIX *)(param_1 + 0x10));
  return;
}

