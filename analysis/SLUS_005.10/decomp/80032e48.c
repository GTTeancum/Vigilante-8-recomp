// addr: 0x80032e48  name: FUN_80032e48

undefined4 * FUN_80032e48(int param_1,int param_2,short param_3)

{
  short sVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int local_28;
  int local_24;
  int local_20;
  
  puVar2 = (undefined4 *)FUN_80031300(param_2,param_1,(int)param_3,0x98,0);
  local_28 = (int)*(short *)((int)puVar2 + 0x12) << 5;
  local_24 = (int)*(short *)(puVar2 + 6) << 5;
  local_20 = (int)*(short *)((int)puVar2 + 0x1e) << 5;
  *puVar2 = 0x800084;
  puVar2[0x19] = &LAB_80032aa4;
  FUN_800202f4(puVar2);
  iVar4 = *(int *)(param_2 + 0x80);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x7f;
  }
  iVar3 = *(short *)((int)puVar2 + 0x12) * 0x6d6;
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  puVar2[0x22] = (iVar4 >> 7) - (iVar3 >> 0xc);
  iVar4 = *(int *)(param_2 + 0x84);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x7f;
  }
  iVar3 = *(short *)(puVar2 + 6) * 0x6d6;
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  puVar2[0x23] = (iVar4 >> 7) - (iVar3 >> 0xc);
  iVar4 = *(int *)(param_2 + 0x88);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x7f;
  }
  iVar3 = *(short *)((int)puVar2 + 0x1e) * 0x6d6;
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  puVar2[0x24] = (iVar4 >> 7) - (iVar3 >> 0xc);
  iVar4 = *(int *)(param_2 + 0xe4);
  if (*(int *)(param_2 + 0xe4) == 0) {
    iVar4 = param_2;
  }
  puVar2[0x21] = iVar4;
  FUN_800176f8(param_2,&local_28,puVar2 + 0x12);
  sVar1 = *(short *)(param_1 + 0xc) + -1;
  *(short *)(param_1 + 0xc) = sVar1;
  if (sVar1 == 0) {
    FUN_8002cb7c(param_1);
  }
  return puVar2;
}

