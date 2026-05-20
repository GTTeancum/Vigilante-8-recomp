// addr: 0x8001f5a0  name: FUN_8001f5a0

int FUN_8001f5a0(int param_1,int param_2)

{
  ushort uVar1;
  MATRIX *m;
  int iVar2;
  short *psVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  short *psVar8;
  int iVar9;
  SVECTOR local_40;
  short local_38;
  short local_36;
  short local_34;
  int local_30;
  
  m = (MATRIX *)FUN_8001d624(*(undefined4 *)(param_2 + 0x10));
  psVar3 = *(short **)(param_2 + 4);
  if (*psVar3 == 1) {
    piVar4 = *(int **)(param_2 + 8);
    psVar8 = psVar3 + 2;
    if ((short)*piVar4 == 1) {
      iVar9 = 0;
      iVar7 = -0x80000000;
      iVar5 = 0;
      do {
        piVar4 = piVar4 + 1;
        if (iVar5 == 0) {
          local_38 = -0x1000;
        }
        else {
          local_38 = (ushort)(iVar5 == 3) << 0xc;
        }
        if (iVar5 == 1) {
          local_36 = -0x1000;
        }
        else {
          local_36 = (ushort)(iVar5 == 4) << 0xc;
        }
        if (iVar5 == 2) {
          local_34 = -0x1000;
        }
        else {
          local_34 = (ushort)(iVar5 == 5) << 0xc;
        }
        local_30 = *piVar4;
        if (iVar5 < 3) {
          local_30 = -local_30;
        }
        iVar6 = FUN_8001e6dc(psVar8,param_1 + 0x10,&local_38,m);
        if (iVar7 < iVar6) {
          iVar9 = iVar5;
          iVar7 = iVar6;
        }
        iVar5 = iVar5 + 1;
      } while (iVar5 < 6);
      uVar1 = (ushort)(iVar9 == 3);
      if (iVar9 == 0) {
        uVar1 = 0xffff;
      }
      local_40.vx = uVar1 << 0xc;
      if (iVar9 == 1) {
        local_40.vy = -0x1000;
      }
      else {
        local_40.vy = (ushort)(iVar9 == 4) << 0xc;
      }
      if (iVar9 == 2) {
        local_40.vz = -0x1000;
      }
      else {
        local_40.vz = (ushort)(iVar9 == 5) << 0xc;
      }
      ApplyMatrixSV(m,&local_40,(SVECTOR *)(param_2 + 0x20));
      FUN_800434f8(param_1 + 0x10,(SVECTOR *)(param_2 + 0x20),param_2 + 0x28);
      if (*(short *)(param_2 + 0x28) < 0) {
        *(undefined4 *)(param_2 + 0x14) = *(undefined4 *)(psVar3 + 8);
      }
      else {
        *(undefined4 *)(param_2 + 0x14) = *(undefined4 *)psVar8;
      }
      if (*(short *)(param_2 + 0x2a) < 0) {
        *(undefined4 *)(param_2 + 0x18) = *(undefined4 *)(psVar3 + 10);
      }
      else {
        *(undefined4 *)(param_2 + 0x18) = *(undefined4 *)(psVar3 + 4);
      }
      if (*(short *)(param_2 + 0x2c) < 0) {
        *(undefined4 *)(param_2 + 0x1c) = *(undefined4 *)(psVar3 + 0xc);
      }
      else {
        *(undefined4 *)(param_2 + 0x1c) = *(undefined4 *)(psVar3 + 6);
      }
      *(int *)(param_2 + 0x30) = iVar7;
    }
    else {
      iVar9 = -0x80000000;
      if ((short)*piVar4 == 2) {
        iVar5 = 0;
        iVar7 = 0;
        if (*(short *)((int)piVar4 + 2) != 0) {
          iVar6 = 4;
          do {
            iVar2 = FUN_8001e6dc(psVar8,param_1 + 0x10,*(int *)(param_2 + 8) + iVar6,m);
            if (iVar9 < iVar2) {
              iVar7 = iVar5;
              iVar9 = iVar2;
            }
            iVar5 = iVar5 + 1;
            iVar6 = iVar6 + 0xc;
          } while (iVar5 < (int)(uint)*(ushort *)(*(int *)(param_2 + 8) + 2));
        }
        ApplyMatrixSV(m,(SVECTOR *)(*(int *)(param_2 + 8) + iVar7 * 0xc + 4),
                      (SVECTOR *)(param_2 + 0x20));
        FUN_800434f8(param_1 + 0x10,(SVECTOR *)(param_2 + 0x20),param_2 + 0x28);
        if (*(short *)(param_2 + 0x28) < 0) {
          *(undefined4 *)(param_2 + 0x14) = *(undefined4 *)(psVar3 + 8);
        }
        else {
          *(undefined4 *)(param_2 + 0x14) = *(undefined4 *)psVar8;
        }
        if (*(short *)(param_2 + 0x2a) < 0) {
          *(undefined4 *)(param_2 + 0x18) = *(undefined4 *)(psVar3 + 10);
        }
        else {
          *(undefined4 *)(param_2 + 0x18) = *(undefined4 *)(psVar3 + 4);
        }
        if (*(short *)(param_2 + 0x2c) < 0) {
          *(undefined4 *)(param_2 + 0x1c) = *(undefined4 *)(psVar3 + 0xc);
        }
        else {
          *(undefined4 *)(param_2 + 0x1c) = *(undefined4 *)(psVar3 + 6);
        }
        *(int *)(param_2 + 0x30) = iVar9;
      }
    }
  }
  return param_2;
}

