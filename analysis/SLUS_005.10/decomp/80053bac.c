// addr: 0x80053bac  name: ResetRCnt

long ResetRCnt(ulong param_1)

{
  long lVar1;
  
  if ((param_1 & 0xffff) < 3) {
    *(undefined2 *)(&TMR_DOTCLOCK_VAL + (param_1 & 0xffff) * 4) = 0;
    lVar1 = COUNTER_OBJ_164();
    return lVar1;
  }
  return 0;
}

