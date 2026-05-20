// addr: 0x8002346c  name: FUN_8002346c

void FUN_8002346c(uint *param_1)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  code *pcVar4;
  undefined2 uVar5;
  int iVar6;
  uint uVar7;
  undefined4 uVar8;
  int iVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  
  if (((iRam0000000c - (uint)*(byte *)((int)param_1 + 9) & 0x7f) == 0) &&
     (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
    iVar10 = 0;
    if (((short)param_1[0x3a] == 0) || (iVar10 = FUN_8001ffd4(&DAT_80065a18), iVar10 == 0)) {
      if ((uint)*(ushort *)(param_1[0x3b] + 0xc) + (uint)*(ushort *)(param_1[0x3c] + 0xc) +
          (uint)*(ushort *)(param_1[0x3d] + 0xc) < (uint)(ushort)param_1[3]) {
        iVar10 = FUN_80023394(&DAT_80065a18,0x100000,param_1 + 9);
        uVar8 = 0x400000;
        if (iVar10 == 0) {
LAB_80023570:
          iVar10 = FUN_80023394(&DAT_80065a18,uVar8,param_1 + 9);
          goto LAB_8002357c;
        }
LAB_800235a0:
        *(undefined2 *)(param_1 + 0x3a) = *(undefined2 *)(iVar10 + 6);
      }
      else {
        uVar8 = 0x7f000000;
        if (param_1[0x45] == 0) goto LAB_80023570;
LAB_8002357c:
        if ((iVar10 != 0) ||
           (iVar10 = FUN_80023394(&DAT_80065a18,0x7f780000,param_1 + 9), iVar10 != 0))
        goto LAB_800235a0;
        iVar10 = FUN_80017160();
        iVar6 = FUN_80020120(&DAT_80065a50,0x7f780000);
        iVar10 = FUN_80020190(&DAT_80065a50,0x7f780000,iVar10 * iVar6 >> 0xf);
      }
      if (iVar10 == 0) {
        FUN_80015368("Nowhere to run!");
      }
    }
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,iVar10 + 0x48,0x22740,0);
  }
  *(undefined1 *)((int)param_1 + 0xb2) = 1;
  sVar2 = FUN_80042f98(param_1,param_1 + 0x30,param_1[0x23] * 0x20 + 0x10000);
  iVar10 = (int)sVar2;
  iVar6 = -0x2aa;
  if (-0x2aa < iVar10) {
    iVar6 = iVar10;
  }
  iVar9 = 0x2aa;
  if (iVar6 < 0x2aa) {
    iVar9 = iVar6;
  }
  *(short *)(param_1 + 0x29) = (short)iVar9;
  iVar6 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
  if (iVar6 < 0) {
    iVar6 = iVar6 + 0xfff;
  }
  iVar3 = (int)(short)param_1[0x2a] + (iVar6 >> 0xc);
  iVar6 = 0;
  if (0 < iVar3) {
    iVar6 = iVar3;
  }
  iVar6 = (short)iVar9 * iVar6;
  if (iVar6 < 0) {
    iVar6 = iVar6 + 0xf;
  }
  param_1[0x25] = param_1[0x25] + (iVar6 >> 4);
  if (iVar10 < 0) {
    iVar10 = -iVar10;
  }
  if ((iVar10 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
    if ((int)param_1[0x23] < 0x1ad3) {
      iVar10 = 0;
      if (0 < *(short *)((int)param_1 + 0xa6)) {
        iVar10 = (int)*(short *)((int)param_1 + 0xa6);
      }
      uVar11 = (uint)(ushort)param_1[0x2b];
      iVar10 = iVar10 + 1;
      bVar1 = iVar10 < (int)uVar11;
      goto LAB_80023778;
    }
    iVar6 = *(short *)((int)param_1 + 0xa6) + -1;
    iVar10 = -(uint)(ushort)param_1[0x2b];
    if ((int)-(uint)(ushort)param_1[0x2b] < iVar6) {
      iVar10 = iVar6;
    }
    *(short *)((int)param_1 + 0xa6) = (short)iVar10;
  }
  else {
    iVar10 = 0;
    if (*(short *)((int)param_1 + 0xa6) < 0) {
      iVar10 = (int)*(short *)((int)param_1 + 0xa6);
    }
    iVar10 = iVar10 + -1;
    uVar11 = -(uint)(ushort)param_1[0x2b];
    bVar1 = (int)uVar11 < iVar10;
LAB_80023778:
    uVar5 = (undefined2)uVar11;
    if (bVar1) {
      uVar5 = (undefined2)iVar10;
    }
    *(undefined2 *)((int)param_1 + 0xa6) = uVar5;
  }
  uVar11 = param_1[*(byte *)((int)param_1 + 0xb3) + 0x44];
  uVar12 = param_1[0x43];
  if (((uVar11 != 0) && (*(short *)(uVar11 + 6) == 0)) &&
     (iVar10 = FUN_80022e90(param_1), iVar10 == 0)) {
    uVar7 = param_1[0x39];
    iVar10 = param_1[9] - *(int *)(uVar7 + 0x24);
    if (iVar10 < 0) {
      iVar10 = -iVar10;
    }
    if (iVar10 < 0x12c000) {
      iVar10 = param_1[10] - *(int *)(uVar7 + 0x28);
      if (iVar10 < 0) {
        iVar10 = -iVar10;
      }
      if (iVar10 < 0x12c000) {
        iVar10 = param_1[0xb] - *(int *)(uVar7 + 0x2c);
        if (iVar10 < 0) {
          iVar10 = -iVar10;
        }
        if (iVar10 < 0x12c000) {
          if (*(code **)(uVar11 + 100) == (code *)0x0) {
            iVar10 = 0;
          }
          else {
            iVar10 = (**(code **)(uVar11 + 100))(uVar11,0xc,param_1);
          }
          FUN_8002ce68(param_1,iVar10);
          if ((iVar10 == 0) || (uVar11 = FUN_80017160(param_1), (uVar11 & 7) == 0)) {
            FUN_8002cf90(param_1,1);
          }
          if (*(char *)(uVar12 + 8) == '\0') {
            if (iVar10 == 0) {
              if (*(code **)(uVar12 + 100) == (code *)0x0) {
                iVar10 = 0;
              }
              else {
                iVar10 = (**(code **)(uVar12 + 100))(uVar12,0xc,param_1);
              }
              uVar8 = 4;
              if (iVar10 != 0) goto LAB_800238e4;
            }
            else {
              uVar8 = 4;
            }
          }
          else {
LAB_800238e4:
            uVar8 = 0xb;
          }
          pcVar4 = *(code **)(uVar12 + 100);
          if (pcVar4 == (code *)0x0) {
            return;
          }
          goto LAB_8002391c;
        }
      }
    }
  }
  pcVar4 = *(code **)(uVar12 + 100);
  if (pcVar4 == (code *)0x0) {
    return;
  }
  uVar8 = 4;
LAB_8002391c:
  (*pcVar4)(uVar12,uVar8,param_1);
  return;
}

