// addr: 0x8003c288  name: FUN_8003c288

undefined4 FUN_8003c288(int param_1,int param_2)

{
  ushort uVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  
  iVar8 = *(int *)(param_2 + 4) - *(int *)(param_1 + 0x48);
  iVar3 = *(int *)(param_2 + 8) - *(int *)(param_1 + 0x4c);
  iVar4 = *(int *)(param_2 + 0xc) - *(int *)(param_1 + 0x50);
  if (-1 < *(char *)(param_1 + 8)) {
    iVar5 = iVar8;
    if (iVar8 < 0) {
      iVar5 = -iVar8;
    }
    iVar7 = iVar3;
    if (iVar3 < 0) {
      iVar7 = -iVar3;
    }
    if (iVar7 < iVar5) {
      iVar7 = iVar5;
    }
    iVar5 = iVar4;
    if (iVar4 < 0) {
      iVar5 = -iVar4;
    }
    if (iVar5 < iVar7) {
      iVar5 = iVar7;
    }
    if (iVar5 < 0x801) {
      return 1;
    }
  }
  iVar5 = iVar8;
  if (iVar8 < 0) {
    iVar5 = iVar8 + 0x1f;
  }
  iVar7 = iVar4;
  if (iVar4 < 0) {
    iVar7 = iVar4 + 7;
  }
  *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + (iVar5 >> 5) + (iVar7 >> 3);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xf;
  }
  *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + (iVar3 >> 4);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  if (iVar8 < 0) {
    iVar8 = iVar8 + 7;
  }
  *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + ((iVar4 >> 5) - (iVar8 >> 3));
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x48);
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_1 + 0x4c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x50);
  uVar6 = 0;
  if ((*(char *)(param_1 + 8) < '\0') &&
     (uVar6 = 0, (iRam0000000c - (uint)*(byte *)(param_1 + 9) & 3) == 0)) {
    iVar3 = FUN_8001d5a0(param_1);
    iVar8 = 0;
    iVar4 = 0x24;
    do {
      iVar5 = *(int *)(iVar3 + iVar4 + 0xec);
      if ((int)*(char *)(iVar5 + 8) == -(int)*(char *)(param_1 + 8)) {
        if (*(code **)(iVar5 + 100) == (code *)0x0) {
          iVar5 = 0;
        }
        else {
          iVar5 = (**(code **)(iVar5 + 100))(iVar5,0xf,param_1);
        }
        if (iVar5 == 0) {
          iVar5 = *(int *)(iVar3 + iVar4 + 0xec);
          uVar1 = *(ushort *)(iVar5 + 0xc);
          if (uVar1 < 99) {
            *(ushort *)(iVar5 + 0xc) = uVar1 + 1;
          }
        }
      }
      iVar8 = iVar8 + 1;
      iVar4 = iVar4 + 4;
    } while (iVar8 < 3);
    sVar2 = *(short *)(param_1 + 0xc) + -1;
    *(short *)(param_1 + 0xc) = sVar2;
    if (sVar2 == 0) {
      uVar6 = FUN_8004410c();
      iVar3 = FUN_8001d624(param_1);
      FUN_8004483c(uVar6,uRam000005f8,0x28,iVar3 + 0x14);
      FUN_8001d564(param_1);
      FUN_800204dc(param_1);
      uVar6 = 0xffffffff;
    }
    else {
      uVar6 = 0;
    }
  }
  return uVar6;
}

