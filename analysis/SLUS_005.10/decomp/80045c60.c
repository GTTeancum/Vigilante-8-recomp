// addr: 0x80045c60  name: SPU_OBJ_78C

undefined4 SPU_OBJ_78C(undefined4 param_1,undefined4 param_2,uint param_3)

{
  uint *unaff_s0;
  
  DAT_8005ee2c = (*unaff_s0 >> 6) + (uint)((*unaff_s0 & 0x3f) != 0);
  DMA_SPU_MADR = unaff_s0[-1];
  DMA_SPU_BCR = DAT_8005ee2c * 0x10000 | 0x10;
  DMA_SPU_CHCR = param_3 | 0x201;
  if (DAT_8005ee24 == 1) {
    DMA_SPU_CHCR = 0x1000200;
  }
  DAT_8005ee28 = unaff_s0[-1];
  return 0;
}

