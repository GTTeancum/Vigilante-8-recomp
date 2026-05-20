// addr: 0x8002a25c  name: FUN_8002a25c

void FUN_8002a25c(short param_1,short param_2,undefined4 param_3)

{
  short *psVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  RECT local_38;
  short local_30 [4];
  
  piVar3 = &DAT_800a1e20;
  if (iRam00000004 == 0) {
    piVar3 = &DAT_800a2324;
  }
  local_38.h = 1;
  local_38.w = 1;
  piVar4 = piVar3 + 1;
  iVar2 = 0;
  if (0 < *piVar3) {
    psVar1 = (short *)((int)piVar3 + 0xe);
    do {
      local_38.x = psVar1[-1] + param_1;
      local_38.y = *psVar1 + param_2;
      StoreImage(&local_38,(u_long *)local_30);
      if (local_30[0] == 0x7fff) {
        FUN_80029e48(piVar4,param_3);
      }
      iVar2 = iVar2 + 1;
      psVar1 = psVar1 + 10;
      piVar4 = piVar4 + 5;
    } while (iVar2 < *piVar3);
  }
  *piVar3 = 0;
  return;
}

