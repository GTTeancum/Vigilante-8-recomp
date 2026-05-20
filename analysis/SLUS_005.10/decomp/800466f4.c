// addr: 0x800466f4  name: SpuRead

/* Possible S_W.OBJ/SpuWrite */

ulong SpuRead(uchar *addr,ulong size)

{
  if (0x7eff0 < size) {
    size = 0x7eff0;
  }
  _spu_Fw(addr,size);
  if (DAT_8005ee0c == 0) {
    DAT_8005ee08 = 0;
  }
  return size;
}

