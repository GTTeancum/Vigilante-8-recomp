// addr: 0x8002d494  name: FUN_8002d494

void FUN_8002d494(uint *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  code *pcVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  
  uVar9 = *(uint *)(param_2 + 8);
  uVar1 = (uint)((uVar9 & 0x40) != 0);
  if ((*param_1 & 0x20000000) != uVar1 << 0x1d) {
    *param_1 = *param_1 ^ 0x20000000;
    if (uVar1 == 0) {
      *(undefined2 *)(param_1[0x3e] + 0x42) = 0;
    }
    else {
      *(undefined2 *)(param_1[0x3e] + 0x42) = 0x800;
    }
    if (uVar1 == 0) {
      *(undefined2 *)(param_1[0x38] + 0x8e) = 0;
    }
    else {
      *(undefined2 *)(param_1[0x38] + 0x8e) = 0x800;
    }
    *(int *)(param_1[0x38] + 0x98) = -*(int *)(param_1[0x38] + 0x98);
    FUN_8001d708(param_1[0x3e]);
  }
  if ((uVar9 & 0x180000) != 0) {
    uVar5 = 0xffffffff;
    if ((uVar9 & 0x80000) != 0) {
      uVar5 = 1;
    }
    iVar2 = FUN_8002cf90(param_1,uVar5);
    *param_1 = *param_1 & 0xefffffff;
    uVar5 = FUN_8004410c();
    uVar6 = 0x15;
    if (iVar2 != 0) {
      uVar6 = 0x16;
    }
    FUN_8004445c(uVar5,uRam000005f8,uVar6);
    if (iVar2 != 0) {
      pcVar3 = *(code **)(param_1[*(byte *)((int)param_1 + 0xb3) + 0x44] + 100);
      if (pcVar3 != (code *)0x0) {
        (*pcVar3)(param_1[*(byte *)((int)param_1 + 0xb3) + 0x44],10,param_1);
      }
    }
  }
  if (((*param_1 & 0x10000000) == 0) &&
     ((iRam0000000c - (uint)*(byte *)((int)param_1 + 9) & 0x3f) == 0)) {
    FUN_8002ea94(param_1,0);
  }
  if ((uVar9 & 0x200000) != 0) {
    uVar1 = 0;
    if (((*param_1 & 0x10000000) != 0) && ((short)param_1[0x2f] < 0x100)) {
      uVar1 = param_1[0x39];
    }
    uVar1 = FUN_8002ed34(param_1,uVar1);
    param_1[0x39] = uVar1;
    *(undefined2 *)(param_1 + 0x2f) = 0;
    *param_1 = *param_1 | 0x10000000;
    uVar5 = FUN_8004410c();
    FUN_8004445c(uVar5,uRam000005f8,0x16);
  }
  if (((uVar9 & 0x40000) != 0) &&
     ((param_1[*(byte *)((int)param_1 + 0xb3) + 0x44] == 0 ||
      (*(short *)(param_1[*(byte *)((int)param_1 + 0xb3) + 0x44] + 6) != 0)))) {
    uVar5 = FUN_8004410c(param_1);
    FUN_8004445c(uVar5,uRam000005f8,0x15);
  }
  FUN_8002ce68(param_1,uVar9 & 4);
  if (*(short *)((int)param_1 + 0xae) == 0) {
    if (((uVar9 & 0x20000) != 0) && (0x100 < *(int *)(param_2 + 4))) {
      iVar8 = 0;
      iVar2 = 0x24;
      do {
        iVar4 = *(int *)((int)param_1 + iVar2 + 0xec);
        if ((iVar4 != 0) && (*(short *)(iVar4 + 0xc) != 0)) {
          iVar7 = 0;
          if (*(code **)(iVar4 + 100) != (code *)0x0) {
            iVar7 = (**(code **)(iVar4 + 100))(iVar4,9,*(undefined4 *)(param_2 + 4));
          }
          if (iVar7 != 0) {
            uVar5 = FUN_8004410c();
            uVar6 = 0x2a;
            if (iVar7 < 0) {
              uVar6 = 0x15;
            }
            FUN_800447e8(uVar5,uRam000005f8,uVar6,param_1 + 9);
            if (0 < iVar7) {
              *(short *)((int)param_1 + 0xae) = (short)iVar7;
            }
            *(undefined4 *)(param_2 + 4) = 0;
            break;
          }
        }
        iVar8 = iVar8 + 1;
        iVar2 = iVar2 + 4;
      } while (iVar8 < 3);
    }
  }
  else {
    *(short *)((int)param_1 + 0xae) = *(short *)((int)param_1 + 0xae) + -1;
  }
  uVar5 = 4;
  if ((uVar9 & 2) != 0) {
    uVar5 = 0xb;
  }
  pcVar3 = *(code **)(param_1[0x43] + 100);
  if (pcVar3 != (code *)0x0) {
    (*pcVar3)(param_1[0x43],uVar5,param_1);
  }
  if ((short)param_1[0x2f] < 0x100) {
    *(short *)(param_1 + 0x2f) = (short)param_1[0x2f] + 8;
  }
  return;
}

