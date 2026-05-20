// addr: 0x8010a0d4  name: FUN_8010a0d4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010a0d4(int param_1)

{
  char cVar1;
  char cVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  
  uVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100cd8,0xb8,0xffffffec,0);
  iVar5 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,5,0x80,8);
  iVar6 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  uVar3 = DAT_80100ce8;
  uVar7 = DAT_80100ce4;
  cVar2 = DAT_8006531d;
  cVar1 = DAT_8006531c;
  *(undefined4 *)(iVar5 + 0x48) = DAT_80100ce0;
  *(undefined4 *)(iVar5 + 0x4c) = uVar7;
  *(undefined4 *)(iVar5 + 0x50) = uVar3;
  FUN_8001dc1c/*0x8001dc1c*/(iVar5);
  *(undefined1 *)(iVar6 + 4) = 0x32;
  *(undefined1 *)(iVar6 + 5) = 0x73;
  *(undefined1 *)(iVar6 + 6) = 100;
  FUN_80019a58/*0x80019a58*/(iVar6,s_SCREEN_ADJUSTMENT_80100cec,&DAT_80100994,2);
  FUN_800190a8/*0x800190a8*/(iVar6);
  do {
    Buffer_StartOTagOther/*0x80011a10*/();
    iVar6 = (int)_DAT_800659d0;
    _DAT_800659d0 = _DAT_800659d0 + 1;
    FUN_8001fcb4/*0x8001fcb4*/(iVar5,iVar6);
    *(short *)(iVar5 + 0x42) = *(short *)(iVar5 + 0x42) + 0x11;
    *(short *)(iVar5 + 0x40) = *(short *)(iVar5 + 0x40) + 0xd;
    FUN_8001d708/*0x8001d708*/(iVar5);
    FUN_8001a4f8/*0x8001a4f8*/(uVar4,0);
    FUN_8001de08/*0x8001de08*/(iVar5);
    DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
    VSync/*0x80047e44*/(0);
    DrawSync/*0x8004f580*/(0);
    func_0x8001a584(uVar4);
    Tick_PadOnly/*0x800126f0*/();
    uVar8 = _DAT_80065930 | _DAT_80065934;
    FUN_80019e20/*0x80019e20*/();
    if (param_1 == 0) {
LAB_8010a3a8:
      if ((uVar8 & 0x50100000) != 0) break;
      if ((uVar8 & 0x8400000) != 0) {
        uVar7 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,8);
        FUN_80019a58/*0x80019a58*/(uVar7,&DAT_80100d00,&DAT_801121cc,2);
        FUN_800190a8/*0x800190a8*/(uVar7);
        param_1 = 1;
      }
    }
    else {
      if ((uVar8 & 0xf0000000) != 0) {
        func_0x8004454c(1,DAT_8011338c,9);
      }
      if (((uVar8 & 0x1000) != 0) && ('\0' < DAT_8006531d)) {
        DAT_8006531d = DAT_8006531d + -1;
      }
      if (((uVar8 & 0x4000) != 0) && (DAT_8006531d < '\x10')) {
        DAT_8006531d = DAT_8006531d + '\x01';
      }
      if (((uVar8 & 0x8000) != 0) && (-0xb < DAT_8006531c)) {
        DAT_8006531c = DAT_8006531c + -1;
      }
      if (((uVar8 & 0x2000) != 0) && (DAT_8006531c < '\x10')) {
        DAT_8006531c = DAT_8006531c + '\x01';
      }
      if ((uVar8 & 0x900000) != 0) {
        DAT_8006531c = cVar1;
        DAT_8006531d = cVar2;
      }
      if ((uVar8 & 0x8d00000) != 0) {
        uVar7 = 8;
        if ((uVar8 & 0x900000) != 0) {
          uVar7 = 0xd;
        }
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,uVar7);
        FUN_8001a0ac/*0x8001a0ac*/(&DAT_801121cc,0);
        param_1 = 0;
        goto LAB_8010a3a8;
      }
    }
  } while ((uVar8 & 0x800000) == 0);
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,(uint)((uVar8 & 0x900000) != 0) << 3);
  func_0x8001a4ac(uVar4);
  FUN_8001af48/*0x8001af48*/(iVar5);
  return;
}

