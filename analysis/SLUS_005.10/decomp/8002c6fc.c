// addr: 0x8002c6fc  name: FUN_8002c6fc

undefined4 FUN_8002c6fc(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  
  if (*(int *)(*(int *)(param_1 + 0x5c) + 0x18) / 2 < *(int *)(param_3 + 8)) {
    iVar5 = 0;
  }
  else {
    iVar5 = 1;
    if (*(int *)(param_3 + 8) < *(int *)(*(int *)(param_1 + 0x5c) + 0xc) / 2) {
      iVar5 = 2;
    }
  }
  uVar4 = (uint)*(ushort *)(param_1 + 0xc);
  if (((param_2 < 0) && (uVar4 != 0)) &&
     (((uRam00000604 & 8) == 0 || (-1 < *(short *)(param_1 + 6))))) {
    iVar1 = uVar4 + param_2;
    if (*(char *)(param_1 + 0xd0) == '\f') {
      if (iVar1 < 1) {
        if (param_4 != 0) {
LAB_8002c8fc:
          if (-0x15 < iVar1) {
            FUN_8002be84(param_1);
            return 0;
          }
          FUN_8002bd84(param_1);
          return 1;
        }
        iVar1 = 1;
      }
      *(short *)(param_1 + 0xc) = (short)iVar1;
    }
    else {
      iVar3 = iVar5 * 4;
      uVar6 = (uint)(*(ushort *)(param_1 + 0xc) >> 1);
      iVar2 = *(int *)(param_1 + iVar3 + 0xec);
      iVar1 = param_2;
      while (iVar2 != 0) {
        iVar3 = *(int *)(param_1 + iVar3 + 0xec);
        iVar1 = (uint)*(ushort *)(iVar3 + 0xc) + iVar1;
        if ((int)((uint)*(ushort *)(iVar3 + 0xc) * (int)*(char *)(iVar3 + 8) + uVar6) / (int)uVar4
            != (int)(iVar1 * *(char *)(iVar3 + 8) + uVar6) / (int)uVar4) {
          FUN_8003fc50(iVar3);
        }
        if (-1 < iVar1) {
          *(short *)(iVar3 + 0xc) = (short)iVar1;
          return 0;
        }
        *(undefined2 *)(iVar3 + 0xc) = 0;
        if (iVar5 == 1) {
          iVar5 = (uint)(*(short *)(*(int *)(param_1 + 0xec) + 0xc) == 0) << 1;
        }
        else {
          iVar5 = 1;
        }
        if (((*(short *)(*(int *)(param_1 + 0xec) + 0xc) == 0) &&
            (*(short *)(*(int *)(param_1 + 0xf0) + 0xc) == 0)) &&
           (*(short *)(*(int *)(param_1 + 0xf4) + 0xc) == 0)) {
          if (param_4 == 0) {
            return 0;
          }
          goto LAB_8002c8fc;
        }
        iVar3 = iVar5 * 4;
        iVar2 = *(int *)(param_1 + iVar3 + 0xec);
      }
    }
  }
  return 0;
}

