// addr: 0x8001e6dc  name: FUN_8001e6dc

int FUN_8001e6dc(int *param_1,int param_2,SVECTOR *param_3,MATRIX *param_4)

{
  uint uVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  uint uVar14;
  uint uVar15;
  uint uVar16;
  uint uVar17;
  SVECTOR local_38;
  int local_30;
  longlong lVar4;
  
  local_30 = param_2;
  ApplyMatrixSV(param_4,param_3,&local_38);
  uVar15 = (int)((uint)(ushort)local_38.vx << 0x10) >> 0x10;
  uVar8 = (*(int *)(local_30 + 0x14) + (*param_1 + param_1[3]) / 2) - param_4->t[0];
  lVar2 = (ulonglong)uVar15 * (ulonglong)uVar8;
  uVar16 = (int)((uint)(ushort)local_38.vy << 0x10) >> 0x10;
  uVar13 = (*(int *)(local_30 + 0x18) + (param_1[1] + param_1[4]) / 2) - param_4->t[1];
  lVar3 = (ulonglong)uVar16 * (ulonglong)uVar13;
  uVar11 = (uint)lVar3;
  uVar14 = (int)((uint)(ushort)local_38.vz << 0x10) >> 0x10;
  uVar17 = (*(int *)(local_30 + 0x1c) + (param_1[2] + param_1[5]) / 2) - param_4->t[2];
  lVar4 = (ulonglong)uVar14 * (ulonglong)uVar17;
  uVar1 = (uint)lVar4;
  iVar7 = *(int *)(param_3 + 1);
  uVar9 = (int)lVar2 + uVar11;
  uVar10 = uVar9 + uVar1;
  iVar12 = (int)((ulonglong)lVar2 >> 0x20) + uVar15 * ((int)uVar8 >> 0x1f) +
           uVar8 * ((int)((uint)(ushort)local_38.vx << 0x10) >> 0x1f) +
           (int)((ulonglong)lVar3 >> 0x20) + uVar16 * ((int)uVar13 >> 0x1f) +
           uVar13 * ((int)((uint)(ushort)local_38.vy << 0x10) >> 0x1f) + (uint)(uVar9 < uVar11) +
           (int)((ulonglong)lVar4 >> 0x20) + uVar14 * ((int)uVar17 >> 0x1f) +
           uVar17 * ((int)((uint)(ushort)local_38.vz << 0x10) >> 0x1f) + (uint)(uVar10 < uVar1);
  uVar11 = uVar10 >> 0xc | iVar12 * 0x100000;
  FUN_8004366c(local_30,iVar7,uVar11,iVar12 >> 0xc);
  FUN_8004316c(&local_38,&local_38);
  iVar12 = (int)local_38.vx * (param_1[3] - *param_1);
  iVar6 = (int)local_38.vy * (param_1[4] - param_1[1]);
  iVar5 = (int)local_38.vz * (param_1[5] - param_1[2]);
  if (iVar12 < 0) {
    iVar12 = -iVar12;
  }
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  if (iVar5 < 0) {
    iVar5 = -iVar5;
  }
  iVar5 = iVar12 + iVar6 + iVar5;
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x1fff;
  }
  return (uVar11 - iVar7) - (iVar5 >> 0xd);
}

