// addr: 0x800455a8  name: SPU_OBJ_D4

undefined4 SPU_OBJ_D4(int param_1)

{
  dword *pdVar1;
  int iVar2;
  undefined2 *puVar3;
  int unaff_s0;
  
  puVar3 = &DAT_800a3220;
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
    *puVar3 = 0;
    param_1 = param_1 + 1;
    puVar3 = puVar3 + 1;
  } while (param_1 < 10);
  if (unaff_s0 == 0) {
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
    iVar2 = 0;
    pdVar1 = &VOICE_00_LEFT_RIGHT;
    do {
      *(undefined2 *)pdVar1 = 0;
      *(undefined2 *)((int)pdVar1 + 2) = 0;
      *(undefined2 *)(pdVar1 + 1) = 0x3fff;
      *(undefined2 *)((int)pdVar1 + 6) = 0x200;
      *(undefined2 *)(pdVar1 + 2) = 0;
      *(undefined2 *)((int)pdVar1 + 10) = 0;
      iVar2 = iVar2 + 1;
      pdVar1 = pdVar1 + 4;
    } while (iVar2 < 0x18);
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

