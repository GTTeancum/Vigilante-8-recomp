// addr: 0x80104550  name: FUN_80104550

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ulonglong FUN_80104550(int *param_1)

{
  short *psVar1;
  ushort uVar2;
  int iVar3;
  undefined4 uVar4;
  int *piVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  int iVar11;
  undefined8 uVar12;
  int local_b0 [12];
  int local_80 [15];
  undefined4 local_44;
  uint local_40;
  int local_3c;
  int *local_38;
  int *local_34;
  int local_30;
  int *local_2c;
  
  local_80[0] = *(int *)*param_1;
  local_80[1] = *(undefined4 *)(*param_1 + 4);
  local_80[2] = *(undefined4 *)(*param_1 + 8);
  local_80[4] = 0;
  local_80[3] = *(int *)*param_1 + param_1[4];
  local_80[5] = *(int *)(*param_1 + 8) + param_1[5];
  local_80[7] = 0;
  local_80[6] = *(int *)param_1[1] + param_1[6];
  local_80[8] = *(int *)(param_1[1] + 8) + param_1[7];
  local_80[9] = *(undefined4 *)param_1[1];
  local_80[10] = *(int *)(param_1[1] + 4);
  piVar5 = local_b0;
  piVar9 = local_80;
  local_80[0xb] = *(undefined4 *)(param_1[1] + 8);
  do {
    iVar7 = piVar9[1];
    iVar8 = piVar9[2];
    iVar6 = piVar9[3];
    *piVar5 = *piVar9;
    piVar5[1] = iVar7;
    piVar5[2] = iVar8;
    piVar5[3] = iVar6;
    piVar9 = piVar9 + 4;
    piVar5 = piVar5 + 4;
  } while (piVar9 != local_80 + 0xc);
  local_40 = 0;
  local_38 = local_80 + 8;
  local_34 = local_80;
  local_30 = 0;
  local_2c = param_1;
  do {
    piVar9 = (int *)param_1[local_40];
    if (piVar9[6] != 0) {
      local_3c = *(int *)piVar9[3];
      local_80[5] = 0;
      local_80[4] = local_2c[4];
      local_80[7] = 0;
      local_80[6] = local_2c[5];
      local_80[0] = local_2c[4];
      local_80[1] = 0;
      local_80[2] = local_2c[5];
      local_80[3] = 0;
      iVar6 = (*(ushort *)((int)piVar9 + 0x16) & 0xfff) * 4;
      iVar11 = (int)*(short *)(iVar6 + -0x7ff9f84a);
      iVar8 = (int)*(short *)(iVar6 + -0x7ff9f84c);
      iVar6 = 0;
      uVar2 = *(ushort *)(*(int *)piVar9[3] + (short)piVar9[5] * 0x1c + 0x36);
      iVar7 = local_30;
      while (local_30 = iVar7, uVar2 != 0xffff) {
        iVar10 = (uint)uVar2 * 0x1c;
        iVar7 = local_3c + iVar10 + 0x1c;
        V8_MemSet/*0x80044efc*/(local_80 + 0xc,0,0x10);
        local_80[8] = iVar11 * *(int *)(iVar7 + 4) + iVar8 * *(int *)(iVar7 + 0xc);
        if (local_80[8] < 0) {
          local_80[8] = local_80[8] + 0xfff;
        }
        local_80[8] = local_80[8] >> 0xc;
        local_80[10] = -iVar8 * *(int *)(iVar7 + 4) + iVar11 * *(int *)(iVar7 + 0xc);
        if (local_80[10] < 0) {
          local_80[10] = local_80[10] + 0xfff;
        }
        local_80[10] = local_80[10] >> 0xc;
        local_80[9] = local_80[0xd];
        local_80[0xb] = local_44;
        local_80[0xc] = local_80[8];
        local_80[0xe] = local_80[10];
        uVar12 = FUN_800171d4/*0x800171d4*/(local_38,local_34);
        iVar3 = FUN_80016a20/*0x80016a20*/(local_38);
        iVar7 = FUN_80016a20/*0x80016a20*/(local_34);
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        iVar7 = (iVar3 >> 0xc) * iVar7;
        iVar7 = __divdi3/*0x8004779c*/((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),iVar7,iVar7 >> 0x1f);
        if (iVar6 < iVar7) {
          local_80[4] = local_80[8];
          local_80[5] = local_80[9];
          local_80[6] = local_80[10];
          local_80[7] = local_80[0xb];
          iVar6 = iVar7;
        }
        iVar7 = local_30;
        uVar2 = *(ushort *)(local_3c + iVar10 + 0x34);
      }
      *(int *)((int)local_b0 + iVar7) = *piVar9 + local_80[4];
      *(int *)((int)local_b0 + iVar7 + 8) = piVar9[2] + local_80[6];
      uVar4 = Terrain_HeightAt/*0x80025400*/(*(int *)((int)local_b0 + iVar7));
      *(undefined4 *)((int)local_b0 + iVar7 + 4) = uVar4;
    }
    if (((*(ushort *)(piVar9 + 4) & 1) == 0) &&
       (iVar6 = 0, ((uint)*(ushort *)(param_1 + 3) & 2 << (local_40 & 0x1f)) == 0)) {
      psVar1 = (short *)((int)piVar9 + 0x12);
      if (0 < *psVar1) {
        iVar7 = 0;
        do {
          iVar8 = iVar7;
          if ((*(ushort *)(piVar9[7] + 8) < *(ushort *)(param_1 + 2)) &&
             (iVar8 = *(int *)((uint)*(ushort *)(piVar9[7] + 10) * 0x34 + _DAT_80065bd4 + 0x24) / 2,
             iVar8 < iVar7)) {
            iVar8 = iVar7;
          }
          iVar6 = iVar6 + 1;
          piVar9 = piVar9 + 1;
          iVar7 = iVar8;
        } while (iVar6 < *psVar1);
      }
    }
    local_40 = local_40 + 1;
    local_30 = local_30 + 0x24;
    local_2c = local_2c + 2;
  } while ((int)local_40 < 2);
  return (ulonglong)CONCAT24(*(undefined2 *)((int)param_1 + 10),_DAT_80065bd4);
}

