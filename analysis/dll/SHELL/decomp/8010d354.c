// addr: 0x8010d354  name: FUN_8010d354

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010d354(undefined4 param_1,int param_2,uint param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 auStack_30 [4];
  undefined2 local_2c;
  short local_28;
  short local_26;
  undefined1 local_1f;
  
  FUN_80017d5c/*0x80017d5c*/(1,param_2,0,0,0,0xf0,0x140,0xf0);
  do {
    if ((param_3 != 0) && (Pad_Tick/*0x800120d4*/(), ((_DAT_80065930 | _DAT_80065934) & param_3) != 0)) {
      return 1;
    }
    VSync/*0x80047e44*/(0);
    uVar2 = 0x140;
    if (param_2 != 0) {
      uVar2 = 0x1e0;
    }
    SetDefDispEnv/*0x8004f198*/(auStack_30,0,DAT_801133f8 * 0xf0,uVar2,0xf0);
    local_1f = (undefined1)param_2;
    local_2c = 0x140;
    local_28 = (short)DAT_8006531c;
    local_26 = (short)DAT_8006531d;
    PutDispEnv/*0x8004fdb0*/(auStack_30);
    iVar1 = SetDispMask/*0x8004f4e8*/(1);
  } while (-1 < iVar1);
  return 1;
}

