// addr: 0x80046c14  name: SpuSetVoiceVolume

void SpuSetVoiceVolume(int vNum,short volL,short volR)

{
  *(ushort *)(&VOICE_00_LEFT_RIGHT + vNum * 4) = volL & 0x7fff;
  *(ushort *)((int)&VOICE_00_LEFT_RIGHT + vNum * 0x10 + 2) = volR & 0x7fff;
  S_SVV_OBJ_60();
  return;
}

