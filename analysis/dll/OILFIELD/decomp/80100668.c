// addr: 0x80100668  name: FUN_80100668

undefined4 FUN_80100668(int param_1,uint param_2,int *param_3)

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
        puVar3 = (uint *)func_0x80040378(*(undefined4 *)(param_1 + 0x98));
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
          puVar3[0xb] = 0;
          puVar3[10] = 0;
          puVar3[9] = 0;
          puVar3[0x19] = 0x80040470;
          puVar3[0x23] = iVar4 + (iVar5 * iVar4 >> 0xf);
          FUN_8001d4f0/*0x8001d4f0*/(param_1);
        }
        *(undefined2 *)(param_1 + 0x80) = *(undefined2 *)(param_1 + 0x82);
      }
      for (iVar5 = *(int *)(param_1 + 0x38); iVar5 != 0; iVar5 = *(int *)(iVar5 + 0x34)) {
        param_3 = *(int **)(iVar5 + 0x8c);
        *(int *)(iVar5 + 0x24) = *(int *)(iVar5 + 0x24) + *(int *)(iVar5 + 0x88);
        *(int *)(iVar5 + 0x28) = *(int *)(iVar5 + 0x28) + (int)param_3;
        *(int *)(iVar5 + 0x2c) = *(int *)(iVar5 + 0x2c) + *(int *)(iVar5 + 0x90);
      }
      FUN_800205f8/*0x800205f8*/(param_1);
      uVar6 = extraout_v1;
    }
    iVar5 = *param_3;
    if (*(byte *)(iVar5 + 4) != uVar6) {
      return 0;
    }
    FUN_800176f8/*0x800176f8*/(iVar5,&DAT_80100090,param_1 + 0x48);
    FUN_8002c3ac/*0x8002c3ac*/(iVar5);
  }
  *(undefined2 *)(param_1 + 0x80) = 0xffff;
  return 0;
}

