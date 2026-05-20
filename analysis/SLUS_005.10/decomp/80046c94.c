// addr: 0x80046c94  name: SpuSetVoicePitch

void SpuSetVoicePitch(int vNum,ushort pitch)

{
  (&VOICE_00_ADPCM_SAMPLE_RATE)[vNum * 8] = pitch;
  S_SVP_OBJ_54();
  return;
}

