// addr: 0x80045864  name: SPU_OBJ_390

void SPU_OBJ_390(void)

{
  int iVar1;
  uint uVar2;
  uint unaff_s0;
  uint unaff_s1;
  word *unaff_s2;
  ushort unaff_s3;
  
  do {
    do {
      unaff_s1 = unaff_s1 - unaff_s0;
      _spu_Fw1ts();
      _spu_Fw1ts();
      if (unaff_s1 == 0) {
        SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf;
        if ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != unaff_s3) {
          uVar2 = 1;
          do {
            if (0xf00 < uVar2) {
              printf("SPU:T/O [%s]\n","wait (dmaf clear/W)");
              SPU_OBJ_424();
              return;
            }
            uVar2 = uVar2 + 1;
          } while ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != unaff_s3);
        }
        return;
      }
      unaff_s0 = 0x40;
      if (unaff_s1 < 0x41) {
        unaff_s0 = unaff_s1;
      }
      iVar1 = 0;
      if (0 < (int)unaff_s0) {
        do {
          SOUND_RAM_DATA_TRANSFER_FIFO = *unaff_s2;
          unaff_s2 = unaff_s2 + 1;
          iVar1 = iVar1 + 2;
        } while (iVar1 < (int)unaff_s0);
      }
      SPU_CTRL_REG_CPUCNT = SPU_CTRL_REG_CPUCNT & 0xffcf | 0x10;
      _spu_Fw1ts();
    } while ((SPU_STATUS_REG_SPUSTAT & 0x400) == 0);
    uVar2 = 1;
    do {
      if (0xf00 < uVar2) {
        printf("SPU:T/O [%s]\n","wait (wrdy H -> L)");
        SPU_OBJ_390();
        return;
      }
      uVar2 = uVar2 + 1;
    } while ((SPU_STATUS_REG_SPUSTAT & 0x400) != 0);
  } while( true );
}

