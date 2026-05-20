// addr: 0x801002bc  name: FUN_801002bc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801002bc(int param_1,uint param_2,int *param_3)

{
  short sVar1;
  uint uVar2;
  uint *puVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint extraout_v1;
  
  uVar6 = 2;
  if (param_2 != 2) {
    uVar2 = 3;
    if (((param_2 < 3) && (uVar2 = 0, param_2 == 0)) || (param_2 != uVar2)) {
      sVar1 = *(short *)(param_1 + 0x80) + -1;
      *(short *)(param_1 + 0x80) = sVar1;
      if (sVar1 == -1) {
        puVar3 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x20,0xa0,8);
        if (puVar3 != (uint *)0x0) {
          uVar6 = FUN_80017160/*0x80017160*/();
          iVar4 = (uVar6 & 0xfff) * 4;
          *puVar3 = *puVar3 | 0x410;
          iVar5 = *(int *)(param_1 + 0x84) * (int)*(short *)(iVar4 + -0x7ff9f84c);
          if (iVar5 < 0) {
            iVar5 = iVar5 + 0xfff;
          }
          puVar3[0x22] = iVar5 >> 0xc;
          iVar5 = *(int *)(param_1 + 0x84) * (int)*(short *)(iVar4 + -0x7ff9f84a);
          if (iVar5 < 0) {
            iVar5 = iVar5 + 0xfff;
          }
          puVar3[0x24] = iVar5 >> 0xc;
          iVar5 = FUN_80017160/*0x80017160*/();
          iVar4 = *(int *)(param_1 + 0x88);
          puVar3[0x12] = 0;
          puVar3[0x23] = iVar4 + (iVar5 * iVar4 >> 0xf);
          puVar3[0x13] = 0;
          puVar3[0x14] = 0;
          puVar3[0x19] = 0x8004042c;
          FUN_8001d4f0/*0x8001d4f0*/(param_1);
        }
        *(undefined2 *)(param_1 + 0x80) = *(undefined2 *)(param_1 + 0x82);
      }
      for (iVar5 = *(int *)(param_1 + 0x38); iVar5 != 0; iVar5 = *(int *)(iVar5 + 0x34)) {
        *(short *)(iVar5 + 0x44) = *(short *)(iVar5 + 0x44) + 0x20;
        *(int *)(iVar5 + 0x48) = *(int *)(iVar5 + 0x48) + *(int *)(iVar5 + 0x88);
        *(int *)(iVar5 + 0x4c) = *(int *)(iVar5 + 0x4c) + *(int *)(iVar5 + 0x8c);
        *(int *)(iVar5 + 0x50) = *(int *)(iVar5 + 0x50) + *(int *)(iVar5 + 0x90);
        if (param_3 != (int *)0x0) {
          FUN_8001d708/*0x8001d708*/(iVar5);
        }
      }
      FUN_800205f8/*0x800205f8*/(param_1);
      uVar6 = extraout_v1;
    }
    if (*(byte *)(*param_3 + 4) != uVar6) {
      return 0;
    }
    FUN_800176f8/*0x800176f8*/(*param_3,&DAT_8010007c,param_1 + 0x48);
  }
  *(undefined2 *)(param_1 + 0x80) = 0xffff;
  return 0;
}

