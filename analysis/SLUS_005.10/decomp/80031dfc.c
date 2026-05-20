// addr: 0x80031dfc  name: FUN_80031dfc

undefined4 * FUN_80031dfc(int param_1,int param_2,short param_3)

{
  undefined4 *puVar1;
  uint *puVar2;
  undefined4 uVar3;
  undefined2 uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  
  puVar1 = (undefined4 *)FUN_80031300(param_2,param_1,(int)param_3,0x98,0);
  puVar2 = (uint *)FUN_8003fd24(puVar1 + 0x12,2);
  *puVar2 = *puVar2 & 0xffffffef;
  uVar6 = puVar1[5];
  uVar7 = puVar1[6];
  uVar8 = puVar1[7];
  puVar2[4] = puVar1[4];
  puVar2[5] = uVar6;
  puVar2[6] = uVar7;
  puVar2[7] = uVar8;
  uVar6 = puVar1[9];
  uVar7 = puVar1[10];
  uVar8 = puVar1[0xb];
  puVar2[8] = puVar1[8];
  puVar2[9] = uVar6;
  puVar2[10] = uVar7;
  puVar2[0xb] = uVar8;
  *puVar1 = 0x800084;
  uVar4 = 0x50;
  if (*(short *)(param_2 + 0x11c) != 0) {
    uVar4 = 0xa0;
  }
  *(undefined2 *)(puVar1 + 3) = uVar4;
  puVar1[0x19] = &LAB_80031bbc;
  FUN_800202f4(puVar1);
  iVar5 = *(int *)(param_2 + 0x80);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  puVar1[0x22] = (iVar5 >> 7) + *(short *)(puVar1 + 5) * 4;
  iVar5 = *(int *)(param_2 + 0x84);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  puVar1[0x23] = (iVar5 >> 7) + *(short *)((int)puVar1 + 0x1a) * 4;
  iVar5 = *(int *)(param_2 + 0x88);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  puVar1[0x24] = (iVar5 >> 7) + *(short *)(puVar1 + 8) * 4;
  *(undefined2 *)(puVar1 + 0x25) = 0xf0;
  uVar3 = FUN_8004410c();
  FUN_800447e8(uVar3,uRam000005f8,0x2f,puVar1 + 0x12);
  FUN_80017594(param_2,&DAT_8006576c,param_1 + 0x48);
  return puVar1;
}

