// addr: 0x8002bc18  name: FUN_8002bc18

void FUN_8002bc18(uint *param_1)

{
  int iVar1;
  undefined4 uVar2;
  short sVar3;
  undefined1 auStack_30 [32];
  
  iVar1 = FUN_8003fbc8();
  if (iVar1 == 0) {
    sVar3 = 0;
  }
  else {
    iVar1 = *(int *)param_1[0x16] + iVar1 * 0x1c + 0x1c;
    uVar2 = FUN_8001d68c(auStack_30,param_1,iVar1);
    FUN_8003f4a0(param_1[0x16],*(undefined2 *)(iVar1 + 0x1a),uVar2);
    sVar3 = 0;
    if (*(short *)(iVar1 + 0x16) != -0x5556) {
      sVar3 = *(short *)(iVar1 + 0x16);
    }
  }
  FUN_80020890(param_1,sVar3);
  param_1[0x19] = (uint)&LAB_8002b98c;
  *(undefined2 *)(param_1 + 3) = 0;
  *(undefined2 *)((int)param_1 + 0xa6) = 0;
  *(undefined2 *)((int)param_1 + 0xbe) = 0;
  *param_1 = *param_1 & 0xffffbfff | 0x8000;
  FUN_800441c8((int)*(char *)((int)param_1 + 5));
  FUN_800441c8(*(undefined1 *)((int)param_1 + 0xd3));
  FUN_80042f5c(param_1 + 0x30);
  iVar1 = iRam000007d0;
  if (*(short *)((int)param_1 + 6) < 1) {
    uRam00000624 = 1;
    *(uint *)param_1[0x38] = *(uint *)param_1[0x38] | 0x20000;
    param_1[0x39] = 0;
  }
  else {
    *(undefined2 *)((int)param_1 + 6) = 0;
    cRam0000067c = cRam0000067c + '\x01';
    if (*(uint **)(iVar1 + 0xe4) == param_1) {
      FUN_8002ea94(iVar1,1);
    }
    if ((iRam000007d4 != 0) && (*(uint **)(iRam000007d4 + 0xe4) == param_1)) {
      FUN_8002ea94(iRam000007d4,1);
    }
  }
  return;
}

