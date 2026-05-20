// addr: 0x8002b940  name: FUN_8002b940

char * FUN_8002b940(int param_1)

{
  if (*(short *)(param_1 + 6) == -1) {
    return "PLAYER 1";
  }
  if (*(short *)(param_1 + 6) != -2) {
    return (&PTR_s_Rattler_800567f0)[(uint)*(byte *)(param_1 + 0xd0) * 2];
  }
  return "PLAYER 2";
}

