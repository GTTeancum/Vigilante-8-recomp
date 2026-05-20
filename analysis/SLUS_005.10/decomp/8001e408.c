// addr: 0x8001e408  name: FUN_8001e408

uint FUN_8001e408(int *param_1,int param_2,SVECTOR *param_3,MATRIX *param_4)

{
  uint uVar1;
  longlong lVar2;
  longlong lVar3;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  uint uVar14;
  SVECTOR local_50;
  int local_48;
  int local_44;
  int local_40;
  SVECTOR *local_38;
  int local_30;
  longlong lVar4;
  
  local_38 = &local_50;
  local_30 = param_2;
  ApplyMatrixSV(param_4,param_3,local_38);
  uVar11 = (int)((uint)(ushort)local_50.vx << 0x10) >> 0x10;
  uVar6 = (*(int *)(local_30 + 0x14) + (*param_1 + param_1[3]) / 2) - param_4->t[0];
  lVar2 = (ulonglong)uVar11 * (ulonglong)uVar6;
  uVar13 = (int)((uint)(ushort)local_50.vy << 0x10) >> 0x10;
  uVar9 = (*(int *)(local_30 + 0x18) + (param_1[1] + param_1[4]) / 2) - param_4->t[1];
  lVar3 = (ulonglong)uVar13 * (ulonglong)uVar9;
  uVar5 = (uint)lVar3;
  uVar10 = (int)((uint)(ushort)local_50.vz << 0x10) >> 0x10;
  uVar14 = (*(int *)(local_30 + 0x1c) + (param_1[2] + param_1[5]) / 2) - param_4->t[2];
  lVar4 = (ulonglong)uVar10 * (ulonglong)uVar14;
  uVar1 = (uint)lVar4;
  uVar7 = (int)lVar2 + uVar5;
  uVar8 = uVar7 + uVar1;
  iVar12 = (uVar8 >> 0xc |
           ((int)((ulonglong)lVar2 >> 0x20) + uVar11 * ((int)uVar6 >> 0x1f) +
            uVar6 * ((int)((uint)(ushort)local_50.vx << 0x10) >> 0x1f) +
            (int)((ulonglong)lVar3 >> 0x20) + uVar13 * ((int)uVar9 >> 0x1f) +
            uVar9 * ((int)((uint)(ushort)local_50.vy << 0x10) >> 0x1f) + (uint)(uVar7 < uVar5) +
            (int)((ulonglong)lVar4 >> 0x20) + uVar10 * ((int)uVar14 >> 0x1f) +
            uVar14 * ((int)((uint)(ushort)local_50.vz << 0x10) >> 0x1f) + (uint)(uVar8 < uVar1)) *
           0x100000) - *(int *)(param_3 + 1);
  if (iVar12 < 0) {
    uVar5 = 1;
  }
  else {
    FUN_800434d0(local_30,local_38,&local_48);
    local_48 = local_48 * (param_1[3] - *param_1);
    local_44 = local_44 * (param_1[4] - param_1[1]);
    local_40 = local_40 * (param_1[5] - param_1[2]);
    if (local_48 < 0) {
      local_48 = -local_48;
    }
    if (local_44 < 0) {
      local_44 = -local_44;
    }
    if (local_40 < 0) {
      local_40 = -local_40;
    }
    local_40 = local_48 + local_44 + local_40;
    if (local_40 < 0) {
      local_40 = local_40 + 0x1fff;
    }
    uVar5 = (uint)(iVar12 - (local_40 >> 0xd)) >> 0x1f;
  }
  return uVar5;
}

