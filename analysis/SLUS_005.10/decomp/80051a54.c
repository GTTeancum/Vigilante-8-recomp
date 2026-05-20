// addr: 0x80051a54  name: SYS_OBJ_2850

uint SYS_OBJ_2850(int param_1)

{
  uint uVar1;
  
  if (param_1 == 0) {
    SYS_OBJ_298C();
    uVar1 = SYS_OBJ_2888();
    return uVar1;
  }
  uVar1 = DAT_8006514c - DAT_80065150 & 0x3f;
  if (uVar1 != 0) {
    SYS_OBJ_24A0();
  }
  if ((((DMA_GPU_CHCR & 0x1000000) != 0) || ((GPU_REG1 & 0x4000000) == 0)) && (uVar1 == 0)) {
    uVar1 = 1;
  }
  return uVar1;
}

