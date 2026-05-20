// addr: 0x800459d0  name: _spu_Fr_

void _spu_Fr_(dword param_1,word param_2,int param_3)

{
  SOUND_RAM_DATA_TRANSFER_ADDR = param_2;
  _spu_Fw1ts();
  SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT | 0x30;
  _spu_Fw1ts();
  SPU_OBJ_AB0();
  DMA_SPU_MADR = param_1;
  DMA_SPU_BCR = param_3 << 0x10 | 0x10;
  DAT_8005ee24 = 1;
  DMA_SPU_CHCR = 0x1000200;
  return;
}

