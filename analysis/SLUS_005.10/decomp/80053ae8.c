// addr: 0x80053ae8  name: COUNTER_OBJ_74

undefined4 COUNTER_OBJ_74(void)

{
  int in_v0;
  ushort in_a3;
  int in_t0;
  
  if (in_v0 != 0) {
    in_a3 = in_a3 | 0x10;
  }
  *(ushort *)(&TMR_DOTCLOCK_MODE + in_t0 * 4) = in_a3;
  return 1;
}

