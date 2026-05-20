// addr: 0x80052254  name: GetClut

u_short GetClut(int x,int y)

{
  return (ushort)(y << 6) | (ushort)(x >> 4) & 0x3f;
}

