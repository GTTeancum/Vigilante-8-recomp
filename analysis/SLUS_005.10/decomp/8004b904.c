// addr: 0x8004b904  name: StClearRing

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void StClearRing(void)

{
  DAT_800a32b4 = 0;
  DAT_800a32b0 = 0;
  DAT_800a32ac = 0;
  DAT_800a32a4 = 0;
  init_ring_status(0,DAT_800a32cc);
  DAT_800a3294 = 0;
  DAT_800a328c = 0;
  DAT_800a3288 = 0;
  return;
}

