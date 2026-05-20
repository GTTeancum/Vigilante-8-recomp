// addr: 0x8010fe88  name: FUN_8010fe88

undefined4
FUN_8010fe88(uint param_1,undefined4 param_2,int param_3,int *param_4,int param_5,int param_6)

{
  undefined4 *puVar1;
  uint uVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  undefined1 auStack_80 [32];
  undefined4 local_60 [10];
  uint local_38;
  int *local_34;
  undefined4 *local_30;
  
  local_34 = param_4;
  if (DAT_80113438 != 0) {
    printf/*0x80052604*/(s_Access_Denied____system_busy_8010124c);
  }
  strcat/*0x800523d4*/(auStack_80,param_2);
  iVar11 = 0;
  iVar10 = 0;
  local_38 = 0;
  DAT_80113434 = DAT_80113434 | 1 << (DAT_80113444 & 0x1f);
  local_38 = param_5 + param_6;
  iVar13 = 0;
  if (0 < (int)local_38) {
    local_30 = local_60 + 8;
    iVar12 = 0;
    do {
      uVar3 = local_38;
      if (iVar10 == 0) {
        do {
          uVar2 = (uint)(0 < param_5);
          if (uVar3 != 0) goto LAB_80110024;
          local_38 = uVar2;
          if (uVar2 == 0) break;
          iVar11 = iVar11 + 1;
          uVar3 = (uint)(iVar11 < 4);
        } while (uVar3 != 0);
        DAT_8011348c = 0;
        if (DAT_80113438 != 0) {
          printf/*0x80052604*/(s_Access_Denied____event_multiple_o_801010e4,local_60);
        }
        DAT_80113438 = 2;
        DAT_8011343c = 0;
        DAT_80113440 = 0;
        DAT_80113444 = param_1;
      }
      uVar2 = (uint)(iVar10 < param_5);
      if (local_38 == 0) break;
LAB_80110024:
      if ((uVar2 == 0) && (param_3 != 0)) {
        puVar8 = local_60;
        puVar1 = (undefined4 *)(iVar12 + param_3);
        do {
          puVar9 = puVar1;
          puVar7 = puVar8;
          uVar4 = puVar7[1];
          uVar5 = puVar7[2];
          uVar6 = puVar7[3];
          *puVar9 = *puVar7;
          puVar9[1] = uVar4;
          puVar9[2] = uVar5;
          puVar9[3] = uVar6;
          puVar8 = puVar7 + 4;
          puVar1 = puVar9 + 4;
        } while (puVar8 != local_30);
        uVar4 = puVar7[5];
        puVar9[4] = *puVar8;
        puVar9[5] = uVar4;
        iVar12 = iVar12 + 0x28;
        iVar13 = iVar13 + 1;
      }
      iVar10 = iVar10 + 1;
      local_38 = (uint)(iVar10 < param_5 + param_6);
    } while (local_38 != 0);
  }
  if (local_34 != (int *)0x0) {
    *local_34 = iVar13;
  }
  return 0;
}

