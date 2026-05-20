// addr: 0x80051e24  name: SYS_OBJ_2C20

undefined4 SYS_OBJ_2C20(void)

{
  int iVar1;
  uint in_v0;
  uint in_v1;
  
  while (((in_v0 & in_v1) != 0 || ((GPU_REG1 & 0x4000000) == 0))) {
    iVar1 = SYS_OBJ_29C0();
    if (iVar1 != 0) {
      return 0xffffffff;
    }
    in_v1 = 0x1000000;
    in_v0 = DMA_GPU_CHCR;
  }
  DMACallback(2,SYS_OBJ_2FBC);
  SYS_OBJ_1C20();
  return 0;
}

