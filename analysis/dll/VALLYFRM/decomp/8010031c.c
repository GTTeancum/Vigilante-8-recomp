// addr: 0x8010031c  name: FUN_8010031c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010031c(uint *param_1,undefined4 param_2,undefined4 *param_3)

{
  undefined2 uVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  
  switch(param_2) {
  case 0:
    if ((param_3 != (undefined4 *)0x0) &&
       (uVar2 = param_1[0x20], param_1[0x20] = uVar2 - (int)param_3, (int)(uVar2 - (int)param_3) < 0
       )) {
      do {
        uVar2 = param_1[0x21];
        puVar3 = (undefined4 *)((uVar2 & 7) * 0x80 + param_1[0x22]);
        puVar7 = puVar3 + 0x20;
        param_1[0x20] = param_1[0x20] + 6;
        puVar6 = (undefined4 *)&DAT_8008f660;
        do {
          uVar5 = puVar3[1];
          uVar8 = puVar3[2];
          uVar9 = puVar3[3];
          *puVar6 = *puVar3;
          puVar6[1] = uVar5;
          puVar6[2] = uVar8;
          puVar6[3] = uVar9;
          puVar3 = puVar3 + 4;
          puVar6 = puVar6 + 4;
        } while (puVar3 != puVar7);
        param_1[0x21] = uVar2 + 1;
      } while ((int)param_1[0x20] < 0);
    }
    DAT_80101304 = FUN_80017160/*0x80017160*/();
  case 0x11:
    if (*(short *)((int)param_1 + 6) == 0x61) {
      iVar4 = FUN_8001fd9c/*0x8001fd9c*/(0x61);
      if (iVar4 != 0) {
        FUN_80024718/*0x80024718*/(iVar4 + 0xc,0x8f80);
      }
    }
    func_0x80022120(param_1,param_3);
  case 1:
    *param_1 = 0x80;
    param_1[0x22] = 0x80;
    uVar5 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,0x100);
    iVar4 = func_0x8003d080(0x7f000000,uVar5);
    _DAT_80065a10 = (uint)(iVar4 != 0);
    DAT_80101308 = 0;
  case 2:
    func_0x80023d00();
    FUN_80020890/*0x80020890*/(param_1,0xf0);
  case 9:
    if (((char)param_1[1] == '\b') && (DAT_80101304 = DAT_80101304 + 1, (DAT_80101304 & 0xf) == 0))
    {
      iVar4 = FUN_8001d5a0/*0x8001d5a0*/(param_1);
      uVar5 = 0x186;
      if ((0xee5 < *(int *)(iVar4 + 0x8c)) && (uVar5 = 0x185, (*param_1 & 0x80000) != 0)) {
        uVar5 = 0x184;
      }
      puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),uVar5,0x80,8);
      *(undefined1 *)(puVar6 + 1) = 1;
      *puVar6 = 0x24;
      uVar5 = param_3[1];
      uVar8 = param_3[2];
      puVar6[0x12] = *param_3;
      puVar6[0x13] = uVar5;
      puVar6[0x14] = uVar8;
      uVar1 = Math_Atan2_Pos/*0x80016c88*/(iVar4 + 0x10);
      *(undefined2 *)((int)puVar6 + 0x42) = uVar1;
      puVar6[0x19] = 0x8003e80c;
      FUN_8002036c/*0x8002036c*/();
      param_1 = (uint *)0x1;
switchD_80100358_caseD_b:
      param_1 = (uint *)FUN_8004410c/*0x8004410c*/(param_1);
      FUN_800443c8/*0x800443c8*/(param_1,*(undefined4 *)(*(int *)(_DAT_800659fc + 0x58) + 8),4,0);
      goto switchD_80100358_caseD_7;
    }
    break;
  case 7:
switchD_80100358_caseD_7:
    FUN_8001d470/*0x8001d470*/(0x8c);
  case 4:
    Heap_Free/*0x80045088*/(param_1[0x22]);
  default:
    break;
  case 0xb:
    goto switchD_80100358_caseD_b;
  }
  return 0;
}

