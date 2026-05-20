// addr: 0x80051d08  name: SYS_OBJ_2B04

undefined4 SYS_OBJ_2B04(uint param_1)

{
  undefined4 uVar1;
  
  GPU_REG1 = 0x10000007;
  if ((GPU_REG0 & 0xffffff) != 2) {
    GPU_REG0 = 0xe1001007;
    uVar1 = SYS_OBJ_2B9C();
    return uVar1;
  }
  if ((param_1 & 8) != 0) {
    GPU_REG1 = 0x9000001;
    uVar1 = SYS_OBJ_2B9C();
    return uVar1;
  }
  return 1;
}

