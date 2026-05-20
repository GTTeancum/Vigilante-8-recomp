// addr: 0x8004b964  name: StSetStream

void StSetStream(u_long mode,u_long start_frame,u_long end_frame,func1 *func1,func2 *func2)

{
  FUN_8004ba34(1);
  DAT_800a32b8 = 0;
  DAT_800a32d0 = func1;
  DAT_800a3290 = mode & 1;
  DAT_800a32a0 = 0;
  DAT_800a3298 = 0;
  DAT_800a328c = 0;
  DAT_800a3288 = 0;
  DAT_800a32d4 = func2;
  return;
}

