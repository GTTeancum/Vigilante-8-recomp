// addr: 0x80045a78  name: _spu_t

undefined4 _spu_t(int param_1,uint param_2,uint param_3)

{
  undefined4 uVar1;
  uint uVar2;
  ushort uVar3;
  
  if (param_1 == 1) {
    DAT_8005ee24 = 0;
    if (SOUND_RAM_DATA_TRANSFER_ADDR != DAT_8005edec) {
      uVar2 = 1;
      do {
        if (0xf00 < uVar2) {
          DAT_8005ee24 = 0;
          return 0xfffffffe;
        }
        uVar2 = uVar2 + 1;
      } while (SOUND_RAM_DATA_TRANSFER_ADDR != DAT_8005edec);
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf | 0x20;
    uVar1 = SPU_OBJ_814();
    return uVar1;
  }
  if (param_1 < 2) {
    if (param_1 != 0) {
      uVar1 = SPU_OBJ_814();
      return uVar1;
    }
    DAT_8005ee24 = 1;
    if (SOUND_RAM_DATA_TRANSFER_ADDR != DAT_8005edec) {
      uVar2 = 1;
      do {
        if (0xf00 < uVar2) {
          DAT_8005ee24 = 1;
          return 0xfffffffe;
        }
        uVar2 = uVar2 + 1;
      } while (SOUND_RAM_DATA_TRANSFER_ADDR != DAT_8005edec);
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 0x30;
    uVar1 = SPU_OBJ_814();
    return uVar1;
  }
  if (param_1 == 2) {
    DAT_8005edec = (word)(param_2 >> (DAT_8005edfc & 0x1f));
    SOUND_RAM_DATA_TRANSFER_ADDR = DAT_8005edec;
    uVar1 = SPU_OBJ_814();
    return uVar1;
  }
  if (param_1 == 3) {
    uVar3 = 0x20;
    if (DAT_8005ee24 == 1) {
      uVar3 = 0x30;
    }
    uVar2 = 1;
    while ((SPU_CTRL_REG_CPUCNT & 0x30) != uVar3) {
      if (0xf00 < uVar2) {
        return 0xfffffffe;
      }
      uVar2 = uVar2 + 1;
    }
    if (DAT_8005ee24 != 1) {
      SPU_OBJ_A88();
      DAT_8005ee2c = (param_3 >> 6) + (uint)((param_3 & 0x3f) != 0);
      DMA_SPU_CHCR = 0x1000201;
      if (DAT_8005ee24 == 1) {
        DMA_SPU_CHCR = 0x1000200;
      }
      DMA_SPU_BCR = DAT_8005ee2c * 0x10000 | 0x10;
      DMA_SPU_MADR = param_2;
      DAT_8005ee28 = param_2;
      return 0;
    }
    SPU_OBJ_AB0();
    uVar1 = SPU_OBJ_78C();
    return uVar1;
  }
  uVar1 = SPU_OBJ_814();
  return uVar1;
}

