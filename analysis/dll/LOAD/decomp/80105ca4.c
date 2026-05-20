// addr: 0x80105ca4  name: FUN_80105ca4

void FUN_80105ca4(undefined4 param_1,short *param_2,int param_3,uint param_4)

{
  int iVar1;
  uint uVar2;
  undefined2 *puVar3;
  int iVar4;
  int iVar5;
  undefined2 *puVar6;
  uint uVar7;
  undefined2 *puVar8;
  undefined2 *puVar9;
  uint uVar10;
  int iVar11;
  short local_48;
  short local_46;
  short local_44;
  short local_42;
  int local_40 [4];
  undefined4 local_30;
  
  uVar10 = (param_4 >> 2 ^ 1) & 1;
  local_40[2] = (int)param_2[1] + (int)param_2[3];
  iVar4 = (int)*(short *)(param_3 + 2) + (int)*(short *)(param_3 + 6);
  iVar11 = (param_4 & 1) + 2;
  if (iVar4 < local_40[2]) {
    local_40[2] = iVar4;
  }
  local_40[3] = (int)param_2[3] + 0xfU & 0xfffffff0;
  iVar1 = Heap_AllocOrRetry/*0x800116f4*/(0x11000);
  local_30 = Heap_CallocOrRetry/*0x8001178c*/(iVar1 + 0x40,4);
  uVar7 = 0;
  iVar5 = (int)*(short *)(param_3 + 2) - (int)param_2[1];
  iVar4 = 0;
  if (0 < iVar5) {
    iVar4 = iVar5;
  }
  iVar4 = iVar4 * iVar11 * 0x10;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 3;
  }
  iVar4 = iVar4 >> 2;
  iVar5 = local_40[3] * iVar11 * 0x10;
  Heap_Free/*0x80045088*/(iVar1,local_30,iVar1);
  local_40[0] = Heap_AllocOrRetry/*0x800116f4*/(iVar5,param_4 & 3);
  if (uVar10 != 0) {
    Heap_AllocOrRetry/*0x800116f4*/(iVar5);
  }
  local_40[1] = 0;
  local_48 = *param_2;
  local_46 = param_2[1];
  if (param_2[1] < *(short *)(param_3 + 2)) {
    local_46 = *(short *)(param_3 + 2);
  }
  iVar1 = (int)local_48;
  local_44 = (short)iVar11 * 8;
  local_42 = (short)local_40[2] - local_46;
  if ((param_4 & 1) != 0) {
    iVar1 = 1;
  }
  iVar1 = iVar1 + param_2[2];
  iVar11 = local_40[3] * iVar11 * 8;
  do {
    if ((uVar10 == 0) || (uVar7 = uVar7 + 1, uVar7 != 1)) {
      DrawSync/*0x8004f580*/(0,iVar11 / 2);
      LoadImage/*0x8004f82c*/(&local_48,local_40[uVar7 & 1] + iVar4 * 4);
      local_48 = local_48 + local_44;
    }
  } while ((int)local_48 < iVar1 - local_44);
  iVar11 = (int)local_48;
  uVar2 = ~uVar7;
  if (iVar1 - iVar11 == (int)local_44) {
    iVar11 = local_40[uVar2 & uVar10] + iVar4 * 4;
    uVar2 = LoadImage/*0x8004f82c*/(&local_48);
  }
  puVar9 = (undefined2 *)local_40[uVar7 & 1];
  local_44 = 1;
  puVar6 = (undefined2 *)(local_40[uVar2 & 1] + iVar4 * 4);
  while (iVar11 < iVar1) {
    puVar8 = puVar6 + 1;
    iVar4 = 0;
    puVar3 = puVar9;
    if (0 < local_42) {
      do {
        *puVar3 = *puVar6;
        iVar4 = iVar4 + 1;
        puVar6 = puVar6 + 0x10;
        puVar3 = puVar3 + 1;
      } while (iVar4 < local_42);
    }
    LoadImage/*0x8004f82c*/(&local_48,puVar9);
    local_48 = local_48 + 1;
    puVar6 = puVar8;
    iVar11 = (int)local_48;
  }
  DrawSync/*0x8004f580*/(0);
  Heap_Free/*0x80045088*/(local_30);
  Heap_Free/*0x80045088*/(local_40[0]);
  Heap_Free/*0x80045088*/(local_40[1]);
  return;
}

