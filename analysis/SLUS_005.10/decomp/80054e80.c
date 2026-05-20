// addr: 0x80054e80  name: _padGetActSize

int _padGetActSize(int param_1)

{
  return ((int)(*(byte *)(param_1 + 0xe3) + 1) >> 1) * 4 +
         ((uint)*(byte *)(param_1 + 0xe9) * 5 + 3 & 0xffc) + 4 + *(int *)(param_1 + 0xec);
}

