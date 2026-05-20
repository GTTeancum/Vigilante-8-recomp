// addr: 0x80101a90  name: FUN_80101a90

undefined4 FUN_80101a90(int param_1,uint param_2)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if (param_2 != 2) {
    uVar2 = 5;
    if (((param_2 < 3) && (uVar2 = 0, param_2 == 0)) || (param_2 != uVar2)) {
      sVar1 = *(short *)(param_1 + 0x80) + -1;
      *(short *)(param_1 + 0x80) = sVar1;
      if (sVar1 == -1) {
        iVar3 = FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x98),*(undefined2 *)(param_1 + 0x96),0x80
                                ,8);
        iVar4 = FUN_80017160/*0x80017160*/();
        iVar5 = *(int *)(param_1 + 0x54);
        *(undefined4 *)(iVar3 + 0x28) = 0;
        *(int *)(iVar3 + 0x24) = (iVar4 * 2 * iVar5 >> 0xf) - iVar5;
        iVar4 = FUN_80017160/*0x80017160*/();
        iVar5 = *(int *)(param_1 + 0x54);
        *(undefined4 *)(iVar3 + 100) = 0x8003e80c;
        *(int *)(iVar3 + 0x2c) = (iVar4 * 2 * iVar5 >> 0xf) - iVar5;
        FUN_8001d4f0/*0x8001d4f0*/(param_1);
        *(undefined2 *)(param_1 + 0x80) = *(undefined2 *)(param_1 + 0x82);
      }
      if (*(int *)(param_1 + 0x38) != 0) {
        return 0;
      }
      FUN_800205f8/*0x800205f8*/(param_1);
    }
    *(undefined4 *)(param_1 + 0x60) = 0;
  }
  *(undefined2 *)(param_1 + 0x80) = 0xffff;
  return 0;
}

