// addr: 0x8002305c  name: FUN_8002305c

void FUN_8002305c(uint *param_1)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  undefined2 uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  
  if (((iRam0000000c - (uint)*(byte *)((int)param_1 + 9) & 0x7f) == 0) &&
     (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,param_1[0x39] + 0x24,0x22740,0);
  }
  *(undefined1 *)((int)param_1 + 0xb2) = 1;
  sVar2 = FUN_80042f98(param_1,param_1 + 0x30,param_1[0x23] * 0x20 + 0x10000);
  iVar5 = (int)sVar2;
  iVar6 = -0x2aa;
  if ((-0x2ab < iVar5) && (iVar6 = 0x2aa, iVar5 < 0x2ab)) {
    iVar6 = iVar5;
  }
  *(short *)(param_1 + 0x29) = (short)iVar6;
  iVar5 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0xfff;
  }
  iVar3 = (int)(short)param_1[0x2a] + (iVar5 >> 0xc);
  iVar5 = 0;
  if (0 < iVar3) {
    iVar5 = iVar3;
  }
  iVar6 = iVar6 * iVar5;
  if (iVar6 < 0) {
    iVar6 = iVar6 + 0xf;
  }
  param_1[0x25] = param_1[0x25] + (iVar6 >> 4);
  iVar6 = (int)sVar2;
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  if ((iVar6 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
    if ((int)param_1[0x23] < 0x1ad3) {
      iVar6 = 0;
      if (0 < *(short *)((int)param_1 + 0xa6)) {
        iVar6 = (int)*(short *)((int)param_1 + 0xa6);
      }
      uVar8 = (uint)(ushort)param_1[0x2b];
      iVar6 = iVar6 + 1;
      bVar1 = iVar6 < (int)uVar8;
      goto LAB_80023248;
    }
    iVar5 = *(short *)((int)param_1 + 0xa6) + -1;
    iVar6 = -(uint)(ushort)param_1[0x2b];
    if ((int)-(uint)(ushort)param_1[0x2b] < iVar5) {
      iVar6 = iVar5;
    }
    *(short *)((int)param_1 + 0xa6) = (short)iVar6;
  }
  else {
    iVar6 = 0;
    if (*(short *)((int)param_1 + 0xa6) < 0) {
      iVar6 = (int)*(short *)((int)param_1 + 0xa6);
    }
    iVar6 = iVar6 + -1;
    uVar8 = -(uint)(ushort)param_1[0x2b];
    bVar1 = (int)uVar8 < iVar6;
LAB_80023248:
    uVar4 = (undefined2)uVar8;
    if (bVar1) {
      uVar4 = (undefined2)iVar6;
    }
    *(undefined2 *)((int)param_1 + 0xa6) = uVar4;
  }
  uVar8 = param_1[*(byte *)((int)param_1 + 0xb3) + 0x44];
  iVar6 = 0;
  bVar1 = false;
  if (uVar8 != 0) {
    bVar1 = *(short *)(uVar8 + 6) == 0;
  }
  if ((bVar1) && (iVar5 = FUN_80022e90(param_1), iVar5 == 0)) {
    if (*(code **)(uVar8 + 100) == (code *)0x0) {
      iVar5 = 0;
    }
    else {
      iVar5 = (**(code **)(uVar8 + 100))(uVar8,0xc,param_1);
    }
    if (iVar5 != 0) {
      iVar6 = 1;
    }
  }
  FUN_8002ce68(param_1,iVar6);
  uVar8 = param_1[0x43];
  if (*(char *)(uVar8 + 8) == '\0') {
    if (iVar6 != 0) {
      uVar7 = 4;
      goto LAB_80023334;
    }
    if (*(code **)(uVar8 + 100) == (code *)0x0) {
      iVar5 = 0;
    }
    else {
      iVar5 = (**(code **)(uVar8 + 100))(uVar8,0xc,param_1);
    }
    uVar7 = 4;
    if (iVar5 == 0) goto LAB_80023334;
  }
  uVar7 = 0xb;
LAB_80023334:
  if (*(code **)(uVar8 + 100) != (code *)0x0) {
    (**(code **)(uVar8 + 100))(uVar8,uVar7,param_1);
  }
  if ((bVar1) && ((iVar6 == 0 || (uVar8 = FUN_80017160(param_1), (uVar8 & 7) == 0)))) {
    FUN_8002cf90(param_1,1);
  }
  return;
}

