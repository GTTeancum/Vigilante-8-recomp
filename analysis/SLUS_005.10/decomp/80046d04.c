// addr: 0x80046d04  name: SpuSetVoiceStartAddr

void SpuSetVoiceStartAddr(int vNum,ulong startAddr)

{
  _spu_FsetRXXa(vNum << 3 | 3);
  S_SVSA_OBJ_50();
  return;
}

