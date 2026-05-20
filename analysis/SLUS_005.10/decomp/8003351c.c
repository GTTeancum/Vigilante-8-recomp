// addr: 0x8003351c  name: FUN_8003351c

undefined4 * FUN_8003351c(int param_1,uint *param_2,short param_3,undefined2 param_4)

{
  short sVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  undefined1 auStack_30 [16];
  
  iVar6 = *(int *)(param_1 + 0x38);
  iVar2 = FUN_8001ac44(DAT_800737d8,4,0x80,8);
  puVar3 = (undefined4 *)FUN_80031300(param_2,iVar6,(int)param_3,0x98,iVar2);
  *puVar3 = 0x800094;
  *(undefined2 *)(puVar3 + 3) = param_4;
  puVar3[0x19] = &LAB_80033290;
  FUN_800202f4(puVar3);
  *(undefined2 *)(puVar3 + 0x25) = 0x3c;
  uVar5 = param_2[0x20];
  if ((int)uVar5 < 0) {
    uVar5 = uVar5 + 0x7f;
  }
  puVar3[0x22] = ((int)uVar5 >> 7) + *(short *)(puVar3 + 5) * 6;
  uVar5 = param_2[0x21];
  if ((int)uVar5 < 0) {
    uVar5 = uVar5 + 0x7f;
  }
  puVar3[0x23] = ((int)uVar5 >> 7) + *(short *)((int)puVar3 + 0x1a) * 6;
  uVar5 = param_2[0x22];
  if ((int)uVar5 < 0) {
    uVar5 = uVar5 + 0x7f;
  }
  puVar3[0x24] = ((int)uVar5 >> 7) + *(short *)(puVar3 + 8) * 6;
  *(undefined1 **)(iVar2 + 100) = &LAB_8003e80c;
  if ((*param_2 & 4) == 0) {
    FUN_800207c4(iVar2);
  }
  FUN_80043358(iVar6 + 0x10,&DAT_80065778,auStack_30);
  FUN_80017594(param_2,auStack_30,param_1 + 0x48);
  uVar4 = FUN_8004410c();
  FUN_800447e8(uVar4,uRam000005f8,0x30,puVar3 + 0x12);
  sVar1 = *(short *)(param_1 + 0xc) + -1;
  *(short *)(param_1 + 0xc) = sVar1;
  if (sVar1 == 0) {
    FUN_8002cb7c(param_1);
  }
  return puVar3;
}

