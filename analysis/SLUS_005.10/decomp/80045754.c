// addr: 0x80045754  name: SPU_OBJ_280

void SPU_OBJ_280(word *param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  ushort uVar4;
  
  SOUND_RAM_DATA_TRANSFER_ADDR = DAT_8005edec;
  uVar4 = SPU_STATUS_REG_SPUSTAT & 0x7ff;
  _spu_Fw1ts();
  for (; param_2 != 0; param_2 = param_2 - uVar3) {
    uVar3 = 0x40;
    if (param_2 < 0x41) {
      uVar3 = param_2;
    }
    iVar1 = 0;
    if (0 < (int)uVar3) {
      do {
        SOUND_RAM_DATA_TRANSFER_FIFO = *param_1;
        param_1 = param_1 + 1;
        iVar1 = iVar1 + 2;
      } while (iVar1 < (int)uVar3);
    }
    SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf | 0x10;
    _spu_Fw1ts();
    if ((SPU_STATUS_REG_SPUSTAT & 0x400) != 0) {
      uVar2 = 1;
      do {
        if (0xf00 < uVar2) {
          printf("SPU:T/O [%s]\n","wait (wrdy H -> L)");
          SPU_OBJ_390();
          return;
        }
        uVar2 = uVar2 + 1;
      } while ((SPU_STATUS_REG_SPUSTAT & 0x400) != 0);
    }
    _spu_Fw1ts();
    _spu_Fw1ts();
  }
  SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf;
  if ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != uVar4) {
    uVar3 = 1;
    do {
      if (0xf00 < uVar3) {
        printf("SPU:T/O [%s]\n","wait (dmaf clear/W)");
        SPU_OBJ_424();
        return;
      }
      uVar3 = uVar3 + 1;
    } while ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != uVar4);
  }
  return;
}

