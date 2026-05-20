// addr: 0x800454d4  name: _spu_init

undefined4 _spu_init(int param_1)

{
  undefined4 uVar1;
  uint uVar2;
  dword *pdVar3;
  int iVar4;
  undefined2 *puVar5;
  
  DMA_DPCR = DMA_DPCR | 0xb0000;
  DAT_8005edf0 = 0;
  DAT_8005edf4 = 0;
  DAT_8005edec = 0;
  SPU_MAIN_VOL_L = 0;
  SPU_MAIN_VOL_R = 0;
  SPU_CTRL_REG_CPUCNT = 0;
  _spu_Fw1ts();
  SPU_MAIN_VOL_L = 0;
  SPU_MAIN_VOL_R = 0;
  if ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != 0) {
    uVar2 = 1;
    do {
      if (0xf00 < uVar2) {
        printf("SPU:T/O [%s]\n","wait (reset)");
        uVar1 = SPU_OBJ_D4(0);
        return uVar1;
      }
      uVar2 = uVar2 + 1;
    } while ((SPU_STATUS_REG_SPUSTAT & 0x7ff) != 0);
  }
  iVar4 = 0;
  puVar5 = &DAT_800a3220;
  DAT_8005edf8 = 2;
  DAT_8005edfc = 3;
  DAT_8005ee00 = 8;
  DAT_8005ee04 = 7;
  SOUND_RAM_DATA_TRANSTER_CTRL = 4;
  SPU_REVERB_OUT_L = 0;
  SPU_REVERB_OUT_R = 0;
  SPU_VOICE_KEY_OFF._0_2_ = 0xffff;
  SPU_VOICE_KEY_OFF._2_2_ = 0xffff;
  SPU_VOICE_CHN_REVERB_MODE._0_2_ = 0;
  SPU_VOICE_CHN_REVERB_MODE._2_2_ = 0;
  do {
    *puVar5 = 0;
    iVar4 = iVar4 + 1;
    puVar5 = puVar5 + 1;
  } while (iVar4 < 10);
  if (param_1 == 0) {
    DAT_8005edec = 0x200;
    SPU_VOICE_CHN_FM_MODE._0_2_ = 0;
    SPU_VOICE_CHN_FM_MODE._2_2_ = 0;
    SPU_VOICE_CHN_NOISE_MODE._0_2_ = 0;
    SPU_VOICE_CHN_NOISE_MODE._2_2_ = 0;
    CD_VOL_L = 0;
    CD_VOL_R = 0;
    EXT_VOL_L = 0;
    EXT_VOL_R = 0;
    SPU_OBJ_280(&DAT_8005ee14,0x10);
    iVar4 = 0;
    pdVar3 = &VOICE_00_LEFT_RIGHT;
    do {
      *(undefined2 *)pdVar3 = 0;
      *(undefined2 *)((int)pdVar3 + 2) = 0;
      *(undefined2 *)(pdVar3 + 1) = 0x3fff;
      *(undefined2 *)((int)pdVar3 + 6) = 0x200;
      *(undefined2 *)(pdVar3 + 2) = 0;
      *(undefined2 *)((int)pdVar3 + 10) = 0;
      iVar4 = iVar4 + 1;
      pdVar3 = pdVar3 + 4;
    } while (iVar4 < 0x18);
    SPU_VOICE_KEY_ON._0_2_ = 0xffff;
    SPU_VOICE_KEY_ON._2_2_ = 0xff;
    _spu_Fw1ts();
    _spu_Fw1ts();
    _spu_Fw1ts();
    _spu_Fw1ts();
    SPU_VOICE_KEY_OFF._0_2_ = 0xffff;
    SPU_VOICE_KEY_OFF._2_2_ = 0xff;
    _spu_Fw1ts();
    _spu_Fw1ts();
    _spu_Fw1ts();
    _spu_Fw1ts();
  }
  DAT_8005ee08 = 1;
  SPU_CTRL_REG_CPUCNT = 0xc000;
  DAT_8005ee0c = 0;
  DAT_8005ee10 = 0;
  return 0;
}

