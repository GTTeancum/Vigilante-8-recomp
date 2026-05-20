// addr: 0x80100a30  name: FUN_80100a30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100a30(int param_1,int param_2,int *param_3)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined8 uVar7;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  short local_30;
  short local_2e;
  short local_2c;
  int local_20;
  int local_1c;
  int local_18;
  
  if ((param_2 == 0) || (param_2 != 3)) {
    local_40 = *(undefined4 *)(param_1 + 0x24);
    local_3c = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x54);
    local_38 = *(undefined4 *)(param_1 + 0x2c);
    iVar1 = FUN_8001d748/*0x8001d748*/(param_1,&local_40,&local_30,0);
    if (iVar1 < local_3c + 0x800) {
      iVar5 = (int)local_30;
      iVar6 = *(int *)(param_1 + 0x80);
      iVar1 = iVar6 * iVar5 + *(int *)(param_1 + 0x84) * (int)local_2e +
              *(int *)(param_1 + 0x88) * (int)local_2c;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x7ff;
      }
      iVar1 = iVar1 >> 0xb;
      iVar3 = iVar5 << 1;
      if (iVar1 < 0) {
        iVar3 = iVar1 * iVar5;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        *(int *)(param_1 + 0x80) = iVar6 - (iVar3 >> 0xc);
        iVar3 = iVar1 * local_2e;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        *(int *)(param_1 + 0x84) = (*(int *)(param_1 + 0x84) - (iVar3 >> 0xc)) / 2;
        iVar1 = iVar1 * local_2c;
        if (iVar1 < 0) {
          iVar1 = iVar1 + 0xfff;
        }
        iVar3 = iVar1 >> 0xc;
        *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - iVar3;
        *(undefined4 *)(param_1 + 0x28) = 1;
      }
      iVar1 = (iVar3 + iVar5) * 0x1e;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      *(int *)(param_1 + 0x80) = iVar6 + (iVar1 >> 0xc);
      iVar1 = local_2c * 0x5a;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + (iVar1 >> 0xc);
      iVar1 = -*(int *)(param_1 + 0x88) * (uint)*(ushort *)(param_1 + 0x94);
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      iVar5 = *(int *)(param_1 + 0x80) * (uint)*(ushort *)(param_1 + 0x94);
      *(short *)(param_1 + 0x8c) = (short)(iVar1 >> 0xc);
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xfff;
      }
      *(short *)(param_1 + 0x90) = (short)(iVar5 >> 0xc);
    }
    *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) + 0x5a;
    param_3 = (int *)(param_1 + 0x10);
    func_0x800172b4(param_3,param_3,param_1 + 0x8c);
    uVar2 = _DAT_80065310 - (uint)*(byte *)(param_1 + 9);
    *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + *(int *)(param_1 + 0x80);
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x84);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + *(int *)(param_1 + 0x88);
    if ((uVar2 & 0xf) != 0) {
      return 0;
    }
    MatrixNormal/*0x8004c934*/(param_3,param_3);
  }
  iVar1 = *param_3;
  uVar4 = (uint)*(byte *)(iVar1 + 4);
  uVar2 = 2;
  if (uVar4 == 7) {
    uVar7 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(iVar1 + 0xc));
    uVar4 = (uint)((ulonglong)uVar7 >> 0x20);
    uVar2 = 0;
    if ((int)uVar7 == 0) {
      return 0;
    }
    iVar1 = 1;
  }
  if (uVar4 == uVar2) {
    local_20 = (*(int *)(iVar1 + 0x24) - *(int *)(param_1 + 0x24)) * 8;
    local_1c = (*(int *)(iVar1 + 0x28) - *(int *)(param_1 + 0x28)) * 8;
    local_18 = (*(int *)(iVar1 + 0x2c) - *(int *)(param_1 + 0x2c)) * 8;
    FUN_8003fc50/*0x8003fc50*/(param_1);
    FUN_800205f8/*0x800205f8*/(param_1);
  }
  FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
  iVar1 = *(int *)(param_1 + 0x80) * (int)(short)param_3[8] +
          *(int *)(param_1 + 0x84) * (int)*(short *)((int)param_3 + 0x22) +
          *(int *)(param_1 + 0x88) * (int)(short)param_3[9];
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7ff;
  }
  iVar1 = iVar1 >> 0xb;
  if (iVar1 < 0) {
    iVar5 = iVar1 * (short)param_3[8];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) - (iVar5 >> 0xc);
    iVar5 = iVar1 * *(short *)((int)param_3 + 0x22);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) - (iVar5 >> 0xc);
    iVar1 = iVar1 * (short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - (iVar1 >> 0xc);
  }
  return 0;
}

