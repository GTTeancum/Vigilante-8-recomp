// addr: 0x80019010  name: FUN_80019010

void FUN_80019010(int param_1,ushort param_2)

{
  *(byte *)(param_1 + 7) = (byte)param_2 & 3 ^ 0x65;
  *(ushort *)(param_1 + 0x10) = *(ushort *)(param_1 + 0x10) & 0xff9f | param_2 & 0x60;
  return;
}

