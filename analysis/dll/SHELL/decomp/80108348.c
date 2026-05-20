// addr: 0x80108348  name: FUN_80108348

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80108348(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined4 uVar8;
  uint uVar9;
  
  uVar2 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100a44,0xb8,0x18,0);
  uVar3 = FUN_80018124/*0x80018124*/(0x10,8,1,1,1,1);
  iVar4 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar4 + 4) = 0x32;
  *(undefined1 *)(iVar4 + 5) = 0x73;
  *(undefined1 *)(iVar4 + 6) = 100;
  FUN_80019a58/*0x80019a58*/(iVar4,s_DIFFICULTY_SETTING_80100a4c,&DAT_80100994,2);
  FUN_800190a8/*0x800190a8*/(iVar4);
  iVar4 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,0,0x80,0);
  *(undefined2 *)(*(int *)(iVar4 + 0x30) + 0x28) = 0x40;
  uVar1 = DAT_80100824;
  uVar8 = DAT_80100820;
  *(undefined4 *)(iVar4 + 0x48) = DAT_8010081c;
  *(undefined4 *)(iVar4 + 0x4c) = uVar8;
  *(undefined4 *)(iVar4 + 0x50) = uVar1;
  FUN_8001dc1c/*0x8001dc1c*/(iVar4);
  FUN_8001d708/*0x8001d708*/(iVar4);
  iVar5 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,1,0x80,8);
  uVar1 = DAT_80100824;
  uVar8 = DAT_80100820;
  iVar7 = (int)DAT_8006531a;
  *(undefined4 *)(iVar5 + 0x48) = DAT_8010081c;
  *(undefined4 *)(iVar5 + 0x4c) = uVar8;
  *(undefined4 *)(iVar5 + 0x50) = uVar1;
  *(short *)(iVar5 + 0x42) = (short)((uint)(iVar7 << 0xc) / 3);
  FUN_8001dc1c/*0x8001dc1c*/(iVar5);
  iVar7 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  do {
    *(uint *)(iVar7 + 4) = *(uint *)(iVar7 + 4) & 0xff000000 | 0x404040;
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111fa8,0);
    FUN_80019a58/*0x80019a58*/(iVar7,(&PTR_s_Unleaded_80111fb0)[DAT_8006531a],&DAT_80111fa8,10);
    if (param_1 != 0) {
      *(undefined1 *)(iVar7 + 4) = 0;
      *(undefined1 *)(iVar7 + 5) = 0x80;
      *(undefined1 *)(iVar7 + 6) = 0;
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801007a8,&DAT_80111fa8,8);
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801007ac,&DAT_80111fa8,9);
    }
    do {
      iVar6 = (int)(((uint)((int)DAT_8006531a << 0xc) / 3 - (uint)*(ushort *)(iVar5 + 0x42)) *
                   0x100000) >> 0x10;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xff;
      }
      *(ushort *)(iVar5 + 0x42) = *(ushort *)(iVar5 + 0x42) + (short)((uint)iVar6 >> 8);
      Buffer_StartOTagOther/*0x80011a10*/();
      iVar6 = (int)_DAT_800659d0;
      _DAT_800659d0 = _DAT_800659d0 + 1;
      FUN_8001fcb4/*0x8001fcb4*/(iVar5,iVar6);
      FUN_8001d708/*0x8001d708*/(iVar5);
      FUN_8001a4f8/*0x8001a4f8*/(uVar2,0);
      FUN_8001de08/*0x8001de08*/(iVar5);
      FUN_8001de08/*0x8001de08*/(iVar4);
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      VSync/*0x80047e44*/(0);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(uVar2);
      Tick_PadOnly/*0x800126f0*/();
      uVar9 = _DAT_80065930 | _DAT_80065934;
    } while (((param_1 == 0) || ((uVar9 & 0xa8d00000) == 0)) && ((uVar9 & 0x58d00000) == 0));
    FUN_80019e20/*0x80019e20*/();
    if (param_1 != 0) {
      uVar8 = 0xd;
      if (((uVar9 & 0x900000) == 0) && (uVar8 = 9, (uVar9 & 0x8400000) != 0)) {
        uVar8 = 8;
      }
      func_0x8004454c(1,DAT_8011338c,uVar8);
      if ((uVar9 & 0x8500000) != 0) {
        param_1 = 0;
      }
      DAT_8006531a = '\x01';
    }
    FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,(uint)((uVar9 & 0x8500000) != 0) << 3);
    if ((uVar9 & 0x50100000) != 0) break;
    if ((uVar9 & 0x8400000) != 0) {
      param_1 = 1;
    }
  } while ((uVar9 & 0x800000) == 0);
  func_0x8001a4ac(uVar2);
  FUN_8001af48/*0x8001af48*/(iVar4);
  FUN_8001af48/*0x8001af48*/(iVar5);
  FUN_800190a8/*0x800190a8*/(iVar7);
  FUN_800183ec/*0x800183ec*/(uVar3);
  return;
}

