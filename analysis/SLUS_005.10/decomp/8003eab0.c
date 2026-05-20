// addr: 0x8003eab0  name: FUN_8003eab0

undefined4 FUN_8003eab0(int param_1,int param_2,int *param_3)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int local_28;
  int local_24;
  int local_20;
  
  if (param_2 == 0) {
    *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + *(int *)(param_1 + 0x88);
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x8c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + *(int *)(param_1 + 0x90);
    FUN_800439b8(param_1 + 0x10,(int)*(short *)(param_1 + 0x80),(int)*(short *)(param_1 + 0x82),
                 (int)*(short *)(param_1 + 0x84));
    iVar3 = *(int *)(param_1 + 0x8c) + 0x5a;
    *(int *)(param_1 + 0x8c) = iVar3;
    if (0 < iVar3) {
      iVar3 = FUN_8001d748(param_1,param_1 + 0x24,0,0);
      if (*(int *)(param_1 + 0x28) <= iVar3) {
        return 0;
      }
      *(int *)(param_1 + 0x28) = iVar3;
      cVar2 = *(char *)(param_1 + 0x87) + -1;
      *(char *)(param_1 + 0x87) = cVar2;
      *(int *)(param_1 + 0x8c) = -*(int *)(param_1 + 0x8c) / 2;
      if (cVar2 == '\0') {
        FUN_800205f8(param_1);
        return 0xffffffff;
      }
    }
  }
  else {
    if (param_2 != 3) {
      return 0;
    }
    iVar3 = *param_3;
    if (*(char *)(iVar3 + 4) != '\x02') {
      return 0;
    }
    FUN_8001f5a0(param_1,param_3);
    iVar1 = *(int *)(param_1 + 0x88) * (int)(short)param_3[8] +
            *(int *)(param_1 + 0x8c) * (int)*(short *)((int)param_3 + 0x22) +
            *(int *)(param_1 + 0x90) * (int)(short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x7ff;
    }
    iVar1 = iVar1 >> 0xb;
    if (-1 < iVar1) {
      return 0;
    }
    local_28 = *(int *)(param_1 + 0x88) << 7;
    local_24 = *(int *)(param_1 + 0x8c) << 7;
    local_20 = *(int *)(param_1 + 0x90) << 7;
    FUN_800176f8(iVar3,&local_28,param_1 + 0x24);
    if (*(short *)(iVar3 + 6) < 0) {
      FUN_80012068(~(int)*(short *)(iVar3 + 6),0xff,2,0x80);
    }
    iVar3 = iVar1 * (short)param_3[8];
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - (iVar3 >> 0xc);
    iVar3 = iVar1 * *(short *)((int)param_3 + 0x22);
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) - (iVar3 >> 0xc);
    iVar1 = iVar1 * (short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) - (iVar1 >> 0xc);
  }
  return 0;
}

