// addr: 0x800467e4  name: SpuIsTransferCompleted

long SpuIsTransferCompleted(long flag)

{
  long lVar1;
  
  if ((DAT_8005ed64 != 1) && (DAT_8005ee08 != 1)) {
    lVar1 = TestEvent(DAT_8005ed5c);
    if (flag == 1) {
      if (lVar1 == 0) {
        do {
          lVar1 = TestEvent(DAT_8005ed5c);
        } while (lVar1 == 0);
        lVar1 = S_ITC_OBJ_88();
        return lVar1;
      }
    }
    else if (lVar1 != 1) {
      return lVar1;
    }
    DAT_8005ee08 = 1;
    return 1;
  }
  lVar1 = S_ITC_OBJ_90();
  return lVar1;
}

