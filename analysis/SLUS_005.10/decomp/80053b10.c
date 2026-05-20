// addr: 0x80053b10  name: GetRCnt

long GetRCnt(ulong param_1)

{
  long lVar1;
  
  if ((param_1 & 0xffff) < 3) {
    lVar1 = COUNTER_OBJ_CC();
    return lVar1;
  }
  return 0;
}

