// addr: 0x80040c40  name: FUN_80040c40

uint * FUN_80040c40(undefined4 param_1,undefined2 param_2,MATRIX *param_3,int *param_4,int param_5)

{
  ushort uVar1;
  uint *puVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int local_38;
  int local_34;
  int local_30;
  
  puVar2 = (uint *)FUN_8001d470(0x9c);
  iVar7 = 0;
  *puVar2 = *puVar2 | 0xa0;
  uVar5 = param_3->t[1];
  uVar6 = param_3->t[2];
  puVar2[0x12] = param_3->t[0];
  puVar2[0x13] = uVar5;
  puVar2[0x14] = uVar6;
  puVar2[0x19] = (uint)&LAB_80040b38;
  puVar2[0x15] = 0x10000;
  FUN_8001d708(puVar2);
  SetRotMatrix(param_3);
  if (0 < param_5) {
    do {
      iVar3 = FUN_8001ac44(param_1,param_2,0xa0,0);
      iVar4 = FUN_80017160();
      local_38 = (iVar4 * 2 * *param_4 >> 0xf) - *param_4;
      iVar4 = FUN_80017160();
      local_34 = (iVar4 * 2 * param_4[1] >> 0xf) - param_4[1];
      iVar4 = FUN_80017160();
      iVar7 = iVar7 + 1;
      local_30 = (iVar4 * (param_4[2] / 2) >> 0xf) + param_4[2];
      FUN_80043248(&local_38,iVar3 + 0x88);
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x80) = uVar1 & 0x1f;
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x82) = uVar1 & 0x1f;
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x84) = uVar1 & 0x1f;
      FUN_80044efc(iVar3 + 0x48,0,0xc);
      iVar4 = FUN_80017160();
      *(int *)(iVar3 + 0x94) = (iVar4 * 0xf >> 0xf) + 0xf;
      FUN_8001d4f0(puVar2);
    } while (iVar7 < param_5);
  }
  return puVar2;
}

