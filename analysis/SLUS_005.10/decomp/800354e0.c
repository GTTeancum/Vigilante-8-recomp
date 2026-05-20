// addr: 0x800354e0  name: FUN_800354e0

undefined4 * FUN_800354e0(int param_1,int param_2,short param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  int iVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  int iVar5;
  
  puVar1 = (undefined4 *)FUN_80031300(param_2,param_1,(int)param_3,0x98,0);
  uVar4 = 0x800080;
  if (puVar1[0x18] != 0) {
    uVar4 = 0x800084;
  }
  *puVar1 = uVar4;
  uVar3 = 0x96;
  if (*(short *)(param_2 + 0x11c) != 0) {
    uVar3 = 300;
  }
  *(undefined2 *)(puVar1 + 3) = uVar3;
  puVar1[0x19] = param_4;
  FUN_800202f4(puVar1);
  FUN_8001dc1c(puVar1);
  *(undefined2 *)(puVar1 + 0x25) = 0;
  iVar5 = *(int *)(param_2 + 0x80);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  iVar2 = (int)*(short *)((int)puVar1 + 0x12);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 3;
  }
  puVar1[0x22] = (iVar5 >> 7) - (iVar2 >> 2);
  iVar5 = *(int *)(param_2 + 0x84);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  iVar2 = (int)*(short *)(puVar1 + 6);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 3;
  }
  puVar1[0x23] = (iVar5 >> 7) - (iVar2 >> 2);
  iVar5 = *(int *)(param_2 + 0x88);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0x7f;
  }
  iVar2 = (int)*(short *)((int)puVar1 + 0x1e);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 3;
  }
  puVar1[0x24] = (iVar5 >> 7) - (iVar2 >> 2);
  FUN_80017594(param_2,&DAT_800657a4,param_1 + 0x48);
  uVar4 = FUN_8004410c();
  FUN_800447e8(uVar4,uRam000005f8,0x31,puVar1 + 0x12);
  return puVar1;
}

