// addr: 0x8002a350  name: FUN_8002a350

undefined4 FUN_8002a350(uint *param_1,int param_2,undefined2 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  uint uVar2;
  
  if (param_2 == 1) {
    uVar2 = param_1[0x13];
    *param_1 = *param_1 | 0x88;
    param_1[0x19] = (uint)&LAB_8002e2bc;
    param_1[0x13] = uVar2 - 0x8000;
    param_1[10] = uVar2 - 0x8000;
    FUN_8002cce8(param_1,*(byte *)((int)param_1 + 3) | 1);
    uVar2 = uRam000007d0;
    *param_1 = (uint)(ushort)*param_1;
    param_1[0x39] = uVar2;
    uVar1 = 0;
  }
  else {
    uVar1 = 0;
    if (param_2 == 7) {
      uVar1 = FUN_8002e630(param_1,param_3,param_4);
    }
  }
  return uVar1;
}

