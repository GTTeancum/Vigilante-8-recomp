// addr: 0x80052214  name: GetTPage

u_short GetTPage(int tp,int abr,int x,int y)

{
  return (ushort)((tp & 3U) << 7) | (ushort)((abr & 3U) << 5) | (ushort)((int)(y & 0x100U) >> 4) |
         (ushort)((int)(x & 0x3ffU) >> 6) | (ushort)((y & 0x200U) << 2);
}

