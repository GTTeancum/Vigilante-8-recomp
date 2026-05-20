// addr: 0x80110210  name: FUN_80110210

undefined4 FUN_80110210(uint param_1,undefined4 param_2,int param_3)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  undefined1 auStack_40 [32];
  int local_20;
  
  piVar6 = &DAT_80113438;
  iVar5 = 0;
  if (DAT_80113438 != 0) {
    printf/*0x80052604*/(s_Access_Denied____system_busy_8010124c);
  }
  strcat/*0x800523d4*/(auStack_40,param_2);
  uVar4 = 1 << (DAT_80113444 & 0x1f);
  DAT_80113434 = DAT_80113434 | uVar4;
  piVar7 = piVar6;
  if ((int)uVar4 < 0) goto LAB_801102c8;
LAB_801102c0:
  uVar4 = 7;
  piVar7 = piVar6;
LAB_801102c8:
  piVar6 = (int *)0x2;
LAB_801102d4:
  do {
    if (-1 < (int)uVar4) {
      return 0;
    }
    DAT_8011348c = uVar4;
    if (*piVar7 != 0) {
      printf/*0x80052604*/(s_Access_Denied____event_multiple_o_801010e4,param_3 << 0x10 | 0x200);
    }
    *piVar7 = 2;
    piVar7[1] = 0;
    piVar7[2] = 0;
    iVar3 = DAT_80113484;
    DAT_80113444 = param_1;
    if ((*piVar7 != 0) || (piVar7[2] != 0)) {
      iVar2 = piVar7[2];
      do {
        bVar1 = iVar2 == 0;
        iVar2 = DAT_80113440;
      } while (bVar1);
      piVar7[2] = 0;
      local_20 = iVar3;
    }
    uVar4 = 3;
    if (local_20 == 0) goto LAB_801102c0;
    if (local_20 != 3) {
      if (local_20 == 2) {
        iVar5 = iVar5 + 1;
        uVar4 = (uint)(iVar5 < 4);
        if (uVar4 != 0) goto LAB_801102d4;
      }
      return 0;
    }
  } while( true );
}

