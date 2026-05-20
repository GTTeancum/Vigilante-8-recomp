// addr: 0x80051a8c  name: SYS_OBJ_2888

undefined4 SYS_OBJ_2888(void)

{
  undefined4 uVar1;
  int iVar2;
  
  if (DAT_8006514c != DAT_80065150) {
    uVar1 = SYS_OBJ_2870();
    return uVar1;
  }
  while (((DMA_GPU_CHCR & 0x1000000) != 0 || ((GPU_REG1 & 0x4000000) == 0))) {
    iVar2 = SYS_OBJ_29C0();
    if (iVar2 != 0) {
      return 0xffffffff;
    }
  }
  uVar1 = SYS_OBJ_297C();
  return uVar1;
}

