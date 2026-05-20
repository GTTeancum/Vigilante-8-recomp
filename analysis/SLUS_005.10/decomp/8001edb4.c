// addr: 0x8001edb4  name: FUN_8001edb4

uint FUN_8001edb4(int param_1,uint *param_2)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  
  bVar1 = false;
  if (*(short *)(param_1 + 6) == *(short *)((int)param_2 + 6)) {
    uVar3 = 0;
  }
  else {
    iVar6 = *(int *)(param_1 + 0x54) + param_2[0x15];
    iVar2 = *(int *)(param_1 + 0x24) - param_2[9];
    if (iVar2 < 0) {
      iVar2 = -iVar2;
    }
    if (iVar2 < iVar6) {
      iVar2 = *(int *)(param_1 + 0x28) - param_2[10];
      if (iVar2 < 0) {
        iVar2 = -iVar2;
      }
      if (iVar2 < iVar6) {
        iVar2 = *(int *)(param_1 + 0x2c) - param_2[0xb];
        if (iVar2 < 0) {
          iVar2 = -iVar2;
        }
        bVar1 = iVar2 < iVar6;
      }
    }
    uVar3 = 0;
    if (bVar1) {
      if ((*param_2 & 0x40) != 0) {
        if (*(int *)(param_1 + 0x74) == 0) {
          *(uint **)(param_1 + 0x74) = param_2;
        }
        else {
          *(uint **)(param_1 + 0x78) = param_2;
        }
      }
      if ((((*param_2 & 0x800) == 0) ||
          (piVar4 = (int *)FUN_8001ecc4(param_1,param_2,param_2 + 4), piVar4 == (int *)0x0)) &&
         (piVar4 = (int *)FUN_8001e9a0(param_1,param_2,param_1 + 0x10,param_2 + 4),
         piVar4 == (int *)0x0)) {
        return 0;
      }
      *piVar4 = (int)param_2;
      uVar5 = FUN_8001e120(param_1,3,piVar4);
      if ((uVar5 == 0) || (uVar3 = uVar5 >> 0x1f, uVar5 == 0xffffffff)) {
        iVar2 = piVar4[1];
        iVar6 = piVar4[3];
        uVar3 = uVar5 >> 0x1f;
        *piVar4 = param_1;
        piVar4[1] = piVar4[2];
        piVar4[2] = iVar2;
        piVar4[3] = piVar4[4];
        piVar4[4] = iVar6;
        iVar2 = FUN_8001e120(param_2,3);
        if (iVar2 < 0) {
          uVar3 = uVar3 | 2;
        }
      }
    }
  }
  return uVar3;
}

