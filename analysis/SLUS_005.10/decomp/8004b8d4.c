// addr: 0x8004b8d4  name: StSetRing

void StSetRing(u_long *ring_addr,u_long ring_size)

{
  DAT_800a32c8 = ring_addr;
  DAT_800a32cc = ring_size;
  StClearRing();
  return;
}

