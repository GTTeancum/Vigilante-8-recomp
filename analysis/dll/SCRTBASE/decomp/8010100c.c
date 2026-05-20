// addr: 0x8010100c  name: FUN_8010100c

undefined4 FUN_8010100c(int param_1,int param_2)

{
  if ((param_2 == 0) || (param_2 != 5)) {
    *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + *(int *)(param_1 + 0x88);
    *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x8c);
    *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + *(int *)(param_1 + 0x90);
    *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) * 0xf80 >> 0xc;
    *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) * 0xf80 >> 0xc;
    *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) * 0xf80 >> 0xc;
    *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + *(short *)(param_1 + 0x96);
    FUN_8001d708/*0x8001d708*/();
  }
  FUN_800205f8/*0x800205f8*/(param_1);
  return 0xffffffff;
}

