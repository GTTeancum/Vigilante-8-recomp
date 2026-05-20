// addr: 0x801002b0  name: FUN_801002b0

undefined4 FUN_801002b0(int param_1,uint param_2,int *param_3)

{
  short sVar1;
  uint uVar2;
  uint *puVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint extraout_v1;
  undefined1 auStack_20 [16];
  
  uVar5 = 2;
  if (param_2 != 2) {
    uVar2 = 3;
    if (((param_2 < 3) && (uVar2 = 0, param_2 == 0)) || (param_2 != uVar2)) {
      sVar1 = *(short *)(param_1 + 0x80) + -1;
      *(short *)(param_1 + 0x80) = sVar1;
      if (sVar1 == -1) {
        puVar3 = (uint *)func_0x80040378(*(undefined4 *)(param_1 + 0x98));
        if (puVar3 != (uint *)0x0) {
          puVar3[0x22] = 0;
          puVar3[0x23] = 0;
          *puVar3 = *puVar3 | 0x410;
          iVar4 = FUN_80017160/*0x80017160*/();
          iVar6 = *(int *)(param_1 + 0x8c);
          puVar3[0xb] = 0;
          puVar3[10] = 0;
          puVar3[9] = 0;
          puVar3[0x19] = 0x80040470;
          puVar3[0x24] = iVar6 + (iVar4 * iVar6 >> 0xf);
          FUN_8001d4f0/*0x8001d4f0*/(param_1);
        }
        *(undefined2 *)(param_1 + 0x80) = *(undefined2 *)(param_1 + 0x82);
      }
      for (iVar4 = *(int *)(param_1 + 0x38); iVar4 != 0; iVar4 = *(int *)(iVar4 + 0x34)) {
        param_3 = (int *)(*(int *)(iVar4 + 0x90) + -0x20);
        *(int *)(iVar4 + 0x24) = *(int *)(iVar4 + 0x24) + *(int *)(iVar4 + 0x88);
        *(int *)(iVar4 + 0x28) = *(int *)(iVar4 + 0x28) + *(int *)(iVar4 + 0x8c);
        *(int *)(iVar4 + 0x2c) = *(int *)(iVar4 + 0x2c) + *(int *)(iVar4 + 0x90);
        *(int *)(iVar4 + 0x8c) = *(int *)(iVar4 + 0x8c) + -0x40;
        *(int **)(iVar4 + 0x90) = param_3;
      }
      FUN_800205f8/*0x800205f8*/(param_1);
      uVar5 = extraout_v1;
    }
    iVar4 = *param_3;
    if (*(byte *)(iVar4 + 4) != uVar5) {
      return 0;
    }
    GTE_RotateLongMat/*0x80043358*/(param_1 + 0x10,&DAT_80100054,auStack_20);
    FUN_800176f8/*0x800176f8*/(iVar4,auStack_20,param_1 + 0x48);
    FUN_8002c3ac/*0x8002c3ac*/(iVar4);
  }
  *(undefined2 *)(param_1 + 0x80) = 0xffff;
  return 0;
}

