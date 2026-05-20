// addr: 0x800467b4  name: SpuSetTransferMode

long SpuSetTransferMode(long mode)

{
  long lVar1;
  
  if ((mode != 0) && (mode == 1)) {
    lVar1 = S_STM_OBJ_1C();
    return lVar1;
  }
  DAT_8005ed64 = mode;
  DAT_8005edf0 = 0;
  return 0;
}

