// addr: 0x80051904  name: SYS_OBJ_2700

undefined4 SYS_OBJ_2700(uint param_1)

{
  undefined4 uVar1;
  uint uVar2;
  
  DAT_8006515c = SetIntrMask(0);
  DAT_80065150 = 0;
  DAT_8006514c = 0;
  uVar2 = param_1 & 7;
  if (uVar2 != 1) {
    if (uVar2 < 2) {
      if (uVar2 != 0) {
        uVar1 = SYS_OBJ_281C();
        return uVar1;
      }
SYS_OBJ_2770:
      DMA_GPU_CHCR = 0x401;
      DMA_DPCR = DMA_DPCR | 0x800;
      GPU_REG1 = 0;
      SYS_OBJ_2FE4(&DAT_800a3340,0,0x100);
      SYS_OBJ_2FE4(&DAT_800a3448,0,0x1800);
      uVar1 = SYS_OBJ_281C();
      return uVar1;
    }
    if (uVar2 != 3) {
      if (uVar2 == 5) goto SYS_OBJ_2770;
      goto code_r0x80051a20;
    }
  }
  DMA_GPU_CHCR = 0x401;
  DMA_DPCR = DMA_DPCR | 0x800;
  GPU_REG1 = 0x1000000;
code_r0x80051a20:
  SetIntrMask(DAT_8006515c);
  uVar1 = 0;
  if ((param_1 & 7) == 0) {
    uVar1 = SYS_OBJ_2B04(param_1);
  }
  return uVar1;
}

