// addr: 0x80104d24  name: FUN_80104d24

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_80104d24(uint param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  uint *puVar7;
  undefined4 uVar8;
  int iVar9;
  undefined *puVar10;
  int iVar11;
  int iVar12;
  int *piVar13;
  uint uVar14;
  undefined1 auStack_d0 [32];
  undefined1 auStack_b0 [16];
  undefined1 auStack_a0 [16];
  undefined1 auStack_90 [32];
  undefined1 auStack_70 [32];
  int local_50 [4];
  undefined4 local_40;
  uint local_3c;
  undefined4 local_38;
  int local_30;
  int *local_2c;
  
  V8_MemSet/*0x80044efc*/(local_50,0,0xc);
  uVar14 = 0;
  AsyncList_RecycleHead/*0x80011834*/();
  local_30 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_CharSel1_tbl_801007b0);
  Async_StopAllocCallback/*0x800165cc*/(0);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(0);
  func_0x80018d00(auStack_90,local_30 + *(int *)(local_30 + 0x38),auStack_b0,0x176,100);
  DrawPrim/*0x8004fb18*/(auStack_90);
  FUN_8001884c/*0x8001884c*/(auStack_b0);
  func_0x80018d00(auStack_90,local_30 + *(int *)(local_30 + 0x3c),auStack_b0,0,0);
  func_0x80018d00(auStack_70,local_30 + *(int *)(local_30 + 0x40),auStack_a0,0,0);
  _DAT_80065a30 = 0x100;
  DAT_800659d2 = 0x80;
  do {
    FUN_80019e20/*0x80019e20*/();
    local_2c = local_50;
    iVar4 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
    *(undefined1 *)(iVar4 + 4) = 0x7c;
    *(undefined1 *)(iVar4 + 5) = 0x60;
    *(undefined1 *)(iVar4 + 6) = 0;
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111e2c,0);
    FUN_80019a58/*0x80019a58*/(iVar4,s_CHOOSE_PLAYER_801007c4,&DAT_80111e2c,10);
    FUN_800190a8/*0x800190a8*/(iVar4);
    func_0x8001d3d8(1,&DAT_801007d4);
    func_0x8001d404(0,&DAT_80100628,0xffffff);
    FUN_80016da8/*0x80016da8*/(auStack_d0);
    FUN_8004d314/*0x8004d314*/(auStack_d0,&DAT_80100798);
    FUN_8001d9c0/*0x8001d9c0*/(auStack_d0,0x200);
    FUN_8001d370/*0x8001d370*/();
    do {
      FUN_8001a994/*0x8001a994*/(_DAT_800737d4);
      iVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_801007f8,0xb8,0xffffffec,0);
      FUN_8001a4f8/*0x8001a4f8*/(iVar4,0);
      iVar11 = 0;
      puVar10 = &DAT_801007d4;
      iVar5 = uVar14 * 0x14;
      if ((&DAT_80111c75)[iVar5] != -1) {
        puVar10 = (undefined *)
                  ((int)**(short **)(iVar4 + 0x14) + (uint)(byte)(&DAT_80111c76)[iVar5]);
        iVar11 = (int)(*(short **)(iVar4 + 0x14))[1] + (uint)(byte)(&DAT_80111c77)[iVar5];
      }
      DrawSync/*0x8004f580*/(0,puVar10,iVar11);
      VSync/*0x80047e44*/(0);
      func_0x8001a584(iVar4);
      func_0x8001a4ac(iVar4);
      uVar6 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100800,0xb8,0xffffffec,0);
      iVar4 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
      FUN_80019e20/*0x80019e20*/();
      FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111e24,0);
      *(undefined1 *)(iVar4 + 4) = 0x80;
      *(undefined1 *)(iVar4 + 5) = 0x80;
      *(undefined1 *)(iVar4 + 6) = 0x80;
      FUN_80019a58/*0x80019a58*/(iVar4,(&PTR_s__67_Rattler_80111c70)[uVar14 * 5],&DAT_80111e24,2);
      FUN_80019a58/*0x80019a58*/(iVar4,(&PTR_s_Chassey_Blue_80111c68)[uVar14 * 5],&DAT_80100808,2);
      *(undefined1 *)(iVar4 + 4) = 0x7c;
      *(undefined1 *)(iVar4 + 5) = 99;
      *(undefined1 *)(iVar4 + 6) = 0x16;
      FUN_80019a58/*0x80019a58*/(iVar4,&DAT_801007a8,&DAT_80111e24,0);
      FUN_80019a58/*0x80019a58*/(iVar4,&DAT_801007ac,&DAT_80111e24,1);
      puVar7 = (uint *)func_0x80021c20(uVar14 & 0xffff);
      if ((int)uVar14 < 0xc) {
        *puVar7 = *puVar7 | 8;
        uVar8 = FUN_8001bda0/*0x8001bda0*/(_DAT_800737d4,0xb);
        FUN_8003e598/*0x8003e598*/(puVar7,uVar8);
      }
      uVar2 = DAT_80100818;
      uVar1 = DAT_80100814;
      puVar7[0x12] = DAT_80100810;
      puVar7[0x13] = uVar1;
      puVar7[0x14] = uVar2;
      FUN_8001d708/*0x8001d708*/(puVar7);
      FUN_8001dc1c/*0x8001dc1c*/(puVar7);
      iVar5 = FUN_8001ac44/*0x8001ac44*/(_DAT_800737d4,10,0x80,0);
      *(undefined2 *)(*(int *)(iVar5 + 0x30) + 0x28) = 0x40;
      *(undefined2 *)(iVar5 + 0x40) = 0xfc00;
      uVar3 = DAT_80100824;
      uVar8 = DAT_80100820;
      *(undefined4 *)(iVar5 + 0x48) = DAT_8010081c;
      *(undefined4 *)(iVar5 + 0x4c) = uVar8;
      *(undefined4 *)(iVar5 + 0x50) = uVar3;
      FUN_8001dc1c/*0x8001dc1c*/(iVar5);
      FUN_8001d708/*0x8001d708*/(iVar5);
      func_0x8004454c(1,DAT_8011338c,4);
      do {
        Buffer_StartOTagOther/*0x80011a10*/();
        RotMatrixY/*0x8004d914*/(0x10,puVar7 + 4);
        if ((code *)puVar7[0x19] != (code *)0x0) {
          (*(code *)puVar7[0x19])(puVar7,0,0);
        }
        if ((*puVar7 & 0x1100000) == 0x100000) {
          uVar8 = 3;
          if ((0x1400 < *(ushort *)((int)puVar7 + 0xa2)) &&
             (uVar8 = 2, *(ushort *)((int)puVar7 + 0xa2) < 0x2001)) {
            uVar8 = 1;
          }
          func_0x8004454c(1,DAT_8011338c,uVar8);
          *puVar7 = *puVar7 | 0x1000000;
        }
        puVar7[0xb] = 0x4000000;
        puVar7[9] = 0x4000000;
        FUN_80019e20/*0x80019e20*/();
        iVar11 = 0;
        piVar13 = local_2c;
        do {
          iVar9 = (uint)*(byte *)(iVar11 + uVar14 * 0x24 + -0x7ffa1580) * 2 - *piVar13;
          if ((iVar9 < -1) || (iVar12 = 1, iVar9 < 2)) {
            iVar12 = -1;
          }
          iVar9 = *piVar13;
          iVar11 = iVar11 + 1;
          *piVar13 = iVar9 + iVar12;
          piVar13 = piVar13 + 1;
        } while (iVar11 < 3);
        FUN_8001a4f8/*0x8001a4f8*/(uVar6,0,0x62,((iVar9 + iVar12) / 2) * 5,auStack_90,auStack_70);
        FUN_8001de08/*0x8001de08*/(puVar7);
        FUN_8001de08/*0x8001de08*/(iVar5);
        DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
        VSync/*0x80047e44*/(0);
        DrawSync/*0x8004f580*/(0);
        func_0x8001a584(uVar6);
        Pad_Tick/*0x800120d4*/();
        SetDispMask/*0x8004f4e8*/(1);
      } while ((_DAT_80065930 & 0xa8d00000) == 0);
      if ((param_2 != 0) && ((_DAT_80065930 & 0x8400000) != 0)) {
        local_40 = 0x4000000;
        local_38 = 0x4000000;
        local_3c = 0x2ff800 - puVar7[0x36];
        puVar7[0x12] = 0x4000000;
        puVar7[0x13] = local_3c;
        puVar7[0x14] = 0x4000000;
        *(undefined2 *)((int)puVar7 + 0x42) = 0x4e3;
        FUN_8001d708/*0x8001d708*/(puVar7);
        Buffer_StartOTagOther/*0x80011a10*/();
        FUN_8001a4f8/*0x8001a4f8*/(uVar6,0);
        FUN_8001de08/*0x8001de08*/(puVar7);
        FUN_8001de08/*0x8001de08*/(iVar5);
        DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
        DrawSync/*0x8004f580*/(0);
        func_0x8001a584(uVar6);
      }
      FUN_8001af48/*0x8001af48*/(iVar5);
      FUN_8001a994/*0x8001a994*/(puVar7[0x16]);
      func_0x8001d490(puVar7);
      FUN_800190a8/*0x800190a8*/(iVar4);
      func_0x8001a4ac(uVar6);
      do {
        iVar4 = (_DAT_80065930 >> 0x1d & 1) + uVar14;
        if ((_DAT_80065930 & 0x80000000) != 0) {
          iVar4 = 0xd;
        }
        uVar14 = (iVar4 + 0xd) % 0xd;
      } while ((param_1 & 1 << (uVar14 & 0x1f)) != 0);
    } while ((_DAT_80065930 & 0x8d00000) == 0);
    FUN_8004445c/*0x8004445c*/(2,DAT_8011338c,0);
    if ((param_2 == 0) || ((_DAT_80065930 & 0x8400000) == 0)) break;
    FUN_80019e20/*0x80019e20*/();
    iVar4 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
    *(undefined1 *)(iVar4 + 4) = 0x7c;
    *(undefined1 *)(iVar4 + 5) = 0x60;
    *(undefined1 *)(iVar4 + 6) = 0;
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111e2c,0);
    FUN_80019a58/*0x80019a58*/(iVar4,s_CHOOSE_ENEMIES_80100828,&DAT_80111e2c,10);
    FUN_800190a8/*0x800190a8*/(iVar4);
  } while ((_DAT_80065930 & 0x100000) != 0);
  Heap_Free/*0x80045088*/(local_30);
  Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
  return uVar14;
}

