// addr: 0x80031454  name: FUN_80031454

undefined4 FUN_80031454(int param_1,int *param_2,undefined2 param_3,int param_4)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  int local_28;
  int local_24;
  int local_20;
  
  if (param_2 != (int *)0x0) {
    iVar3 = *param_2;
    if (*(char *)(iVar3 + 4) == '\x03') {
      return 0;
    }
    iVar1 = param_4 << 0x10;
    if (*(char *)(iVar3 + 4) != '\x02') goto LAB_800315c4;
    uVar4 = ((uint)*(ushort *)(param_1 + 0xc) << 0xb) / (uint)*(ushort *)(iVar3 + 0xa2);
    iVar1 = *(int *)(param_1 + 0x88) * uVar4;
    if (iVar1 < -0x80000) {
      local_28 = -0x80000;
    }
    else {
      local_28 = 0x80000;
      if (iVar1 < 0x80001) {
        local_28 = iVar1;
      }
    }
    iVar1 = *(int *)(param_1 + 0x8c) * uVar4;
    local_24 = -0x80000;
    if ((-0x80001 < iVar1) && (local_24 = 0x80000, iVar1 < 0x80001)) {
      local_24 = iVar1;
    }
    iVar1 = *(int *)(param_1 + 0x90) * uVar4;
    local_20 = -0x80000;
    if ((-0x80001 < iVar1) && (local_20 = 0x80000, iVar1 < 0x80001)) {
      local_20 = iVar1;
    }
    FUN_800176f8(iVar3,&local_28,param_1 + 0x24);
    if (*(short *)(iVar3 + 6) < 0) {
      FUN_80012068(~(int)*(short *)(iVar3 + 6),0xff,2,0x80);
    }
    iVar1 = param_4 << 0x10;
    if (*(short *)(iVar3 + 0x11e) == 0) goto LAB_800315c4;
    param_4 = -1;
  }
  iVar1 = param_4 << 0x10;
LAB_800315c4:
  if (-1 < iVar1 >> 0x10) {
    uVar2 = FUN_8004410c();
    FUN_8004483c(uVar2,uRam000005f8,iVar1 >> 0x10,param_1 + 0x24);
  }
  FUN_8003fd24(param_1 + 0x24,param_3);
  (*pcRam00000730)(param_1,0x11,0);
  FUN_800205f8(param_1);
  return 0xffffffff;
}

