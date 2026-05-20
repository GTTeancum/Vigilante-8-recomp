// addr: 0x80025800  name: FUN_80025800

undefined2 * FUN_80025800(int param_1,int param_2,undefined2 *param_3)

{
  undefined2 uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  int iVar16;
  uint uVar17;
  uint uVar18;
  int iVar19;
  
  iVar9 = param_1 + -0x8000;
  iVar4 = param_2 + -0x8000;
  iVar7 = iVar9;
  if (iVar9 < 0) {
    iVar7 = param_1 + 0x7fff;
  }
  uVar18 = iVar7 >> 0x10;
  iVar9 = iVar9 + uVar18 * -0x10000;
  iVar7 = iVar4;
  if (iVar4 < 0) {
    iVar7 = param_2 + 0x7fff;
  }
  uVar13 = iVar7 >> 0x10;
  iVar4 = iVar4 + uVar13 * -0x10000;
  uVar8 = uVar13 >> 6;
  uVar2 = uVar18 >> 6;
  uVar3 = uVar18 + 1 >> 6;
  iVar11 = (uVar13 & 0x3f) * 2;
  iVar19 = (uVar18 & 0x3f) * 0x80;
  iVar16 = (uVar18 + 1 & 0x3f) * 0x80;
  uVar5 = uVar13 + 1 >> 6;
  iVar6 = (uVar13 + 1 & 0x3f) * 2;
  uVar15 = *(ushort *)((&DAT_800911a0)[uVar3 * 0x20 + uVar8] + iVar11 + iVar16) & 0x7ff;
  uVar10 = uVar18 + 2 >> 6;
  iVar14 = 0x10000 - iVar9;
  uVar17 = *(ushort *)((&DAT_800911a0)[uVar2 * 0x20 + uVar8] + iVar11 + iVar19) & 0x7ff;
  iVar7 = (uVar18 + 2 & 0x3f) * 0x80;
  uVar12 = *(ushort *)((&DAT_800911a0)[uVar2 * 0x20 + uVar5] + iVar6 + iVar19) & 0x7ff;
  uVar18 = *(ushort *)((&DAT_800911a0)[uVar3 * 0x20 + uVar5] + iVar6 + iVar16) & 0x7ff;
  iVar11 = (uVar15 - uVar17) * iVar14 +
           ((*(ushort *)((&DAT_800911a0)[uVar10 * 0x20 + uVar8] + iVar11 + iVar7) & 0x7ff) - uVar15)
           * iVar9;
  if (iVar11 < 0) {
    iVar11 = iVar11 + 0xffff;
  }
  iVar7 = (uVar18 - uVar12) * iVar14 +
          ((*(ushort *)((&DAT_800911a0)[uVar10 * 0x20 + uVar5] + iVar6 + iVar7) & 0x7ff) - uVar18) *
          iVar9;
  iVar6 = 0x10000 - iVar4;
  if (iVar7 < 0) {
    iVar7 = iVar7 + 0xffff;
  }
  iVar7 = (iVar11 >> 0x10) * iVar6 + (iVar7 >> 0x10) * iVar4;
  uVar1 = (undefined2)((uint)iVar7 >> 0x10);
  if (iVar7 < 0) {
    uVar1 = (undefined2)((uint)(iVar7 + 0xffff) >> 0x10);
  }
  *param_3 = uVar1;
  uVar5 = uVar13 + 2 >> 6;
  param_3[1] = 0xffe0;
  iVar7 = (uVar13 + 2 & 0x3f) * 2;
  iVar11 = (uVar12 - uVar17) * iVar6 +
           ((*(ushort *)((&DAT_800911a0)[uVar2 * 0x20 + uVar5] + iVar7 + iVar19) & 0x7ff) - uVar12)
           * iVar4;
  if (iVar11 < 0) {
    iVar11 = iVar11 + 0xffff;
  }
  iVar7 = (uVar18 - uVar15) * iVar6 +
          ((*(ushort *)((&DAT_800911a0)[uVar3 * 0x20 + uVar5] + iVar7 + iVar16) & 0x7ff) - uVar18) *
          iVar4;
  if (iVar7 < 0) {
    iVar7 = iVar7 + 0xffff;
  }
  iVar7 = (iVar11 >> 0x10) * iVar14 + (iVar7 >> 0x10) * iVar9;
  uVar1 = (undefined2)((uint)iVar7 >> 0x10);
  if (iVar7 < 0) {
    uVar1 = (undefined2)((uint)(iVar7 + 0xffff) >> 0x10);
  }
  param_3[2] = uVar1;
  return param_3;
}

