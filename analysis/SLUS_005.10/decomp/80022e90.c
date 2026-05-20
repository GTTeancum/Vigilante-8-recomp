// addr: 0x80022e90  name: FUN_80022e90

undefined4 FUN_80022e90(int param_1)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  
  if (cRam00000015 == '\0') {
    if (*(byte *)(param_1 + 0xd0) < 6) {
      return 0;
    }
    iVar5 = *(int *)(param_1 + (uint)*(byte *)(param_1 + 0xb3) * 4 + 0x110);
    if (iVar5 != 0) {
      if (*(char *)(iVar5 + 8) != '\x04') {
        return 0;
      }
      local_28 = *(int *)(param_1 + 0x24) + -0xfa000;
      local_24 = *(int *)(param_1 + 0x24) + 0xfa000;
      local_20 = *(int *)(param_1 + 0x2c) + -0xfa000;
      local_1c = *(int *)(param_1 + 0x2c) + 0xfa000;
      iVar2 = FUN_80021a30(uRam000006fc,(int)sRam000006f0,(int)sRam000007dc,&local_28);
      if (iVar2 != 0) {
        iVar4 = *(int *)(param_1 + 0xe4);
        iVar3 = *(int *)(iVar2 + 0x24) - *(int *)(iVar4 + 0x24);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        iVar3 = *(int *)(iVar2 + 0x28) - *(int *)(iVar4 + 0x28);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        iVar3 = *(int *)(iVar2 + 0x2c) - *(int *)(iVar4 + 0x2c);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        *(int *)(param_1 + 0xe4) = iVar2;
        if (*(code **)(iVar5 + 100) == (code *)0x0) {
          sVar1 = 0;
        }
        else {
          sVar1 = (**(code **)(iVar5 + 100))(iVar5,0xb,param_1);
        }
        iVar2 = FUN_80017160();
        *(short *)(iVar5 + 6) = sVar1 + (short)(iVar2 * 0x80 * (2 - cRam00000016) >> 0xf);
        *(int *)(param_1 + 0xe4) = iVar4;
        return 1;
      }
    }
  }
  return 0;
}

