// addr: 0x80100624  name: FUN_80100624

void FUN_80100624(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  short local_10;
  short local_e;
  short local_c;
  short local_a;
  
  iVar3 = *(int *)(param_1 + 0x5c);
  iVar2 = *(int *)(param_1 + 0x48) + *(int *)(iVar3 + 4);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xffff;
  }
  local_10 = (short)((uint)iVar2 >> 0x10);
  iVar2 = *(int *)(param_1 + 0x50) + *(int *)(iVar3 + 0xc);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xffff;
  }
  local_e = (short)((uint)iVar2 >> 0x10);
  iVar2 = *(int *)(param_1 + 0x48) + *(int *)(iVar3 + 0x10);
  iVar1 = iVar2 + 0xffff;
  if (iVar1 < 0) {
    iVar1 = iVar2 + 0x1fffe;
  }
  local_c = (short)((uint)iVar1 >> 0x10) - local_10;
  iVar2 = *(int *)(param_1 + 0x50) + *(int *)(iVar3 + 0x18);
  iVar3 = iVar2 + 0xffff;
  if (iVar3 < 0) {
    iVar3 = iVar2 + 0x1fffe;
  }
  local_a = (short)((uint)iVar3 >> 0x10) - local_e;
  FUN_80024718/*0x80024718*/(&local_10);
  return;
}

