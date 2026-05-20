// addr: 0x80042f98  name: FUN_80042f98

int FUN_80042f98(int param_1,short *param_2,int param_3)

{
  short sVar1;
  int iVar2;
  long lVar3;
  uint uVar4;
  long local_18 [4];
  
  iVar2 = *(int *)(param_2 + 4) - *(int *)(param_1 + 0x24);
  if (iVar2 < 0) {
    iVar2 = -iVar2;
  }
  if (iVar2 < param_3) {
    iVar2 = *(int *)(param_2 + 6) - *(int *)(param_1 + 0x2c);
    if (iVar2 < 0) {
      iVar2 = -iVar2;
    }
    if (iVar2 < param_3) {
      if (0 < *param_2) {
        sVar1 = param_2[1];
        param_2[1] = sVar1 + 1U;
        uVar4 = *(uint *)(((int)((uint)(ushort)(sVar1 + 1U) << 0x10) >> 0xe) + *(int *)(param_2 + 2)
                         );
        if (uVar4 != 0) {
          *(uint *)(param_2 + 4) = uVar4 << 0x10;
          *(uint *)(param_2 + 6) = uVar4 & 0xffff0000;
          goto LAB_80043048;
        }
      }
      *param_2 = -1;
    }
  }
LAB_80043048:
  local_18[1] = 0;
  local_18[0] = *(int *)(param_2 + 4) - *(int *)(param_1 + 0x24);
  local_18[2] = *(int *)(param_2 + 6) - *(int *)(param_1 + 0x2c);
  FUN_8004352c(param_1 + 0x10,local_18,local_18);
  lVar3 = ratan2(local_18[0],local_18[2]);
  return (lVar3 << 0x14) >> 0x14;
}

