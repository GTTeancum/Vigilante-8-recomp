// addr: 0x80100940  name: FUN_80100940

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100940(uint *param_1,uint param_2,uint *param_3)

{
  short sVar1;
  undefined2 uVar2;
  undefined1 uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  uint uVar7;
  uint uVar8;
  
  if (param_2 != 7) {
    if ((param_2 < 8) && (param_2 == 1)) goto LAB_80100bbc;
    if (param_2 != 9) {
      return 0;
    }
    if ((*param_1 & 1) != 0) {
      return 0;
    }
    param_3 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],*(undefined2 *)((int)param_1 + 10),0x98,0);
    iVar4 = FUN_80017160/*0x80017160*/();
    sVar1 = *(short *)((int)param_1 + 0x42);
    FUN_8001dc1c/*0x8001dc1c*/(param_3);
    iVar5 = param_3[0x15] * 0x93c;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    param_3[0x15] = iVar5 >> 0xc;
    *(undefined2 *)((int)param_3 + 6) = 1000;
    uVar3 = FUN_80017160/*0x80017160*/();
    *(undefined1 *)((int)param_3 + 9) = uVar3;
    uVar2 = *(undefined2 *)((int)param_1 + 0xe);
    *param_3 = *param_3 | 0x488;
    *(undefined2 *)((int)param_3 + 0xe) = uVar2;
    *(undefined2 *)(param_3 + 3) = uVar2;
    uVar7 = param_1[0x13];
    uVar8 = param_1[0x14];
    param_3[0x12] = param_1[0x12];
    param_3[0x13] = uVar7;
    param_3[0x14] = uVar8;
    param_3[0x19] = (uint)FUN_80100244;
    param_3[0x21] = 0xfffff415;
    iVar5 = (((int)sVar1 + ((iVar4 << 8) >> 0xf)) - 0x80U & 0xfff) * 4;
    iVar4 = *(short *)(iVar5 + -0x7ff9f84c) * 0xbeb;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_3[0x20] = iVar4 >> 0xc;
    iVar4 = *(short *)(iVar5 + -0x7ff9f84a) * 0xbeb;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_3[0x22] = iVar4 >> 0xc;
    iVar4 = param_3[0x15] * 0x3243;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    *(short *)(param_3 + 0x25) = (short)(0x1000000 / (iVar4 >> 0xc));
    FUN_8003e76c/*0x8003e76c*/(param_3);
    FUN_8001d708/*0x8001d708*/(param_3);
    FUN_800202f4/*0x800202f4*/(param_3);
    FUN_8001b0c4/*0x8001b0c4*/(param_3);
    uVar6 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar6,_DAT_800658fc,9,param_3 + 9);
    FUN_80020890/*0x80020890*/(param_1,300);
    FUN_8001fe50/*0x8001fe50*/(&DAT_801012a0,param_3);
    DAT_801012ac = DAT_801012ac + 1;
  }
  iVar4 = FUN_8001d470/*0x8001d470*/(0x80);
  *(uint **)(iVar4 + 0x58) = param_1;
  *(short *)(iVar4 + 10) = (short)param_3;
LAB_80100bbc:
  *param_1 = *param_1 | 0x22;
  return 0;
}

