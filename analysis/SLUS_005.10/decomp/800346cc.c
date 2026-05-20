// addr: 0x800346cc  name: FUN_800346cc

undefined4 *
FUN_800346cc(int param_1,uint *param_2,short param_3,undefined2 param_4,undefined2 param_5)

{
  char cVar1;
  uint *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint *puVar7;
  int iVar8;
  undefined1 auStack_30 [16];
  
  iVar8 = *(int *)(param_1 + 0x38);
  puVar2 = (uint *)FUN_8001ac44(DAT_800737d8,10,0x80,8);
  puVar3 = (undefined4 *)FUN_80031300(param_2,iVar8,(int)param_3,0x98,puVar2);
  *(undefined2 *)((int)puVar3 + 10) = param_4;
  *puVar3 = 0x800490;
  *(undefined2 *)(puVar3 + 3) = param_5;
  puVar3[0x19] = &LAB_8003403c;
  FUN_800202f4(puVar3);
  *(undefined2 *)(puVar3 + 0x25) = 0;
  puVar7 = (uint *)param_2[0x39];
  if ((uint *)param_2[0x39] == (uint *)0x0) {
    puVar7 = param_2;
  }
  puVar3[0x21] = puVar7;
  iVar4 = FUN_80016aac(puVar3 + 9,puVar7 + 9);
  iVar4 = iVar4 >> 9;
  iVar5 = 0x1000;
  if ((0xfff < iVar4) && (iVar5 = 0x2000, iVar4 < 0x2001)) {
    iVar5 = iVar4;
  }
  uVar6 = param_2[0x20];
  if ((int)uVar6 < 0) {
    uVar6 = uVar6 + 0x7f;
  }
  iVar4 = *(short *)((int)puVar3 + 0x12) * iVar5;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0xfff;
  }
  puVar3[0x22] = ((int)uVar6 >> 7) - (iVar4 >> 0xc);
  uVar6 = param_2[0x21];
  if ((int)uVar6 < 0) {
    uVar6 = uVar6 + 0x7f;
  }
  iVar4 = *(short *)(puVar3 + 6) * iVar5;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0xfff;
  }
  puVar3[0x23] = ((int)uVar6 >> 7) - (iVar4 >> 0xc);
  uVar6 = param_2[0x22];
  if ((int)uVar6 < 0) {
    uVar6 = uVar6 + 0x7f;
  }
  iVar5 = *(short *)((int)puVar3 + 0x1e) * iVar5;
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0xfff;
  }
  puVar3[0x24] = ((int)uVar6 >> 7) - (iVar5 >> 0xc);
  FUN_80016da8(puVar3 + 4);
  puVar2[0x19] = (uint)&LAB_8003e80c;
  *puVar2 = *puVar2 | 0x410;
  if ((*param_2 & 4) == 0) {
    FUN_800207c4(puVar2);
  }
  FUN_80043358(iVar8 + 0x10,&DAT_80065788,auStack_30);
  FUN_80017594(param_2,auStack_30,param_1 + 0x48);
  cVar1 = FUN_8004410c();
  *(char *)((int)puVar3 + 5) = cVar1;
  FUN_800447e8((int)cVar1,uRam000005f8,0x34,puVar3 + 0x12);
  return puVar3;
}

