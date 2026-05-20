// addr: 0x80051a74  name: SYS_OBJ_2870

undefined4 SYS_OBJ_2870(void)

{
  int iVar1;
  undefined4 uVar2;
  
  SYS_OBJ_24A0();
  iVar1 = SYS_OBJ_29C0();
  if (iVar1 == 0) {
    if (DAT_8006514c != DAT_80065150) {
      uVar2 = SYS_OBJ_2870();
      return uVar2;
    }
    do {
      if (((DMA_GPU_CHCR & 0x1000000) == 0) && ((GPU_REG1 & 0x4000000) != 0)) {
        uVar2 = SYS_OBJ_297C();
        return uVar2;
      }
      iVar1 = SYS_OBJ_29C0();
    } while (iVar1 == 0);
  }
  return 0xffffffff;
}

