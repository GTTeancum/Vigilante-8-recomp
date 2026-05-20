// addr: 0x800516a4  name: SYS_OBJ_24A0

uint SYS_OBJ_24A0(void)

{
  uint uVar1;
  
  uVar1 = 1;
  if ((DMA_GPU_CHCR & 0x1000000) == 0) {
    DAT_80065158 = SetIntrMask(0);
    if (DAT_8006514c != DAT_80065150) {
      do {
        if ((DMA_GPU_CHCR & 0x1000000) != 0) break;
        if (((DAT_80065150 + 1 & 0x3f) == DAT_8006514c) && (DAT_80065030 == (code *)0x0)) {
          DMACallback(2,0);
        }
        do {
        } while ((GPU_REG1 & 0x4000000) == 0);
        (**(code **)(&DAT_800a3448 + DAT_80065150 * 0x60))
                  (*(undefined4 *)(&DAT_800a344c + DAT_80065150 * 0x60),
                   *(undefined4 *)(&DAT_800a3450 + DAT_80065150 * 0x60));
        DAT_80065150 = DAT_80065150 + 1 & 0x3f;
      } while (DAT_8006514c != DAT_80065150);
    }
    SetIntrMask(DAT_80065158);
    if ((((DAT_8006514c == DAT_80065150) && ((DMA_GPU_CHCR & 0x1000000) == 0)) &&
        (DAT_8006502c != 0)) && (DAT_80065030 != (code *)0x0)) {
      DAT_8006502c = 0;
      (*DAT_80065030)();
    }
    uVar1 = DAT_8006514c - DAT_80065150 & 0x3f;
  }
  return uVar1;
}

