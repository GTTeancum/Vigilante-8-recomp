// addr: 0x80017ba8  name: FUN_80017ba8

void FUN_80017ba8(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined8 uVar5;
  uint local_28;
  uint local_24;
  uint local_20;
  
  uVar5 = FUN_80017240(param_1 + 0x80,param_2 + 0x20);
  uVar2 = (uint)uVar5 >> 0xf | (int)((ulonglong)uVar5 >> 0x20) << 0x11;
  if ((int)uVar2 < 0) {
    FUN_800434d0(param_1 + 0x10,param_2 + 0x20,&local_28);
    uVar1 = -*(int *)(param_2 + 0x30);
    uVar3 = uVar1 + uVar2 * -2;
    iVar4 = (((int)uVar1 >> 0x1f) - ((int)(uVar2 * 2) >> 0x1f)) - (uint)(uVar1 < uVar2 * 2);
    local_28 = (uint)((ulonglong)local_28 * (ulonglong)uVar3) >> 0xc |
               ((int)((ulonglong)local_28 * (ulonglong)uVar3 >> 0x20) + local_28 * iVar4 +
               uVar3 * ((int)local_28 >> 0x1f)) * 0x100000;
    local_24 = (uint)((ulonglong)local_24 * (ulonglong)uVar3) >> 0xc |
               ((int)((ulonglong)local_24 * (ulonglong)uVar3 >> 0x20) + local_24 * iVar4 +
               uVar3 * ((int)local_24 >> 0x1f)) * 0x100000;
    local_20 = (uint)((ulonglong)local_20 * (ulonglong)uVar3) >> 0xc |
               ((int)((ulonglong)local_20 * (ulonglong)uVar3 >> 0x20) + local_20 * iVar4 +
               uVar3 * ((int)local_20 >> 0x1f)) * 0x100000;
    FUN_80017594(param_1,&local_28,param_2 + 0x14);
  }
  return;
}

