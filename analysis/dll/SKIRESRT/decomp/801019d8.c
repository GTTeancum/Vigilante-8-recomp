// addr: 0x801019d8  name: FUN_801019d8

undefined4 FUN_801019d8(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 local_10;
  undefined2 local_e;
  undefined2 local_c;
  undefined2 local_a;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  uVar2 = 0;
  if (iVar1 != 0) {
    iVar1 = *(int *)(param_1 + 0x48);
    local_10 = (undefined2)((uint)iVar1 >> 0x10);
    if (iVar1 < 0) {
      local_10 = (undefined2)((uint)(iVar1 + 0xffff) >> 0x10);
    }
    iVar1 = *(int *)(param_1 + 0x50);
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xffff;
    }
    local_e = (undefined2)((uint)iVar1 >> 0x10);
    local_c = 1;
    local_a = 1;
    FUN_80024718/*0x80024718*/(&local_10,0);
    uVar2 = 0xffffffff;
  }
  return uVar2;
}

