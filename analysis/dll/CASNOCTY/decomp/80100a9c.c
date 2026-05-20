// addr: 0x80100a9c  name: FUN_80100a9c

undefined4 FUN_80100a9c(int param_1,int param_2)

{
  short sVar1;
  uint *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  
  if ((param_2 == 0) &&
     (sVar1 = *(short *)(param_1 + 0x80) + -1, *(short *)(param_1 + 0x80) = sVar1, sVar1 == -1)) {
    puVar2 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x98),*(undefined2 *)(param_1 + 0x96)
                                     ,0xa0,8);
    uVar3 = FUN_80017160/*0x80017160*/();
    iVar4 = (uVar3 & 0xfff) * 4;
    *puVar2 = *puVar2 | 0x4b4;
    iVar5 = *(int *)(param_1 + 0x84) * (int)*(short *)(iVar4 + -0x7ff9f84c);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    puVar2[0x22] = iVar5 >> 0xc;
    iVar5 = *(int *)(param_1 + 0x84) * (int)*(short *)(iVar4 + -0x7ff9f84a);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    puVar2[0x24] = iVar5 >> 0xc;
    iVar5 = FUN_80017160/*0x80017160*/();
    puVar2[0x23] = *(int *)(param_1 + 0x88) + (iVar5 * *(int *)(param_1 + 0x88) >> 0xf);
    iVar5 = FUN_8001d624/*0x8001d624*/(param_1);
    uVar3 = *(uint *)(iVar5 + 0x18);
    uVar6 = *(uint *)(iVar5 + 0x1c);
    puVar2[9] = *(uint *)(iVar5 + 0x14);
    puVar2[10] = uVar3;
    puVar2[0xb] = uVar6;
    puVar2[0x19] = 0x800404c4;
    FUN_800202f4/*0x800202f4*/();
    *(undefined2 *)(param_1 + 0x80) = *(undefined2 *)(param_1 + 0x82);
  }
  return 0;
}

