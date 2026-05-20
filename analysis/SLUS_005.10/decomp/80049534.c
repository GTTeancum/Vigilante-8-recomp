// addr: 0x80049534  name: CdPosToInt

int CdPosToInt(CdlLOC *p)

{
  return (((uint)(p->minute >> 4) * 10 + (p->minute & 0xf)) * 0x3c +
         (uint)(p->second >> 4) * 10 + (p->second & 0xf)) * 0x4b +
         (uint)(p->sector >> 4) * 10 + (p->sector & 0xf) + -0x96;
}

