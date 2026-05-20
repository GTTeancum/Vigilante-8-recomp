// addr: 0x8003ce24  name: FUN_8003ce24

uint FUN_8003ce24(uint param_1)

{
  int iVar1;
  uint in_v1;
  int iVar2;
  
  iVar2 = ~(int)cRam00000016 + 3;
  while( true ) {
    if (iVar2 == -1) {
      return in_v1;
    }
    do {
      do {
        iVar1 = FUN_80017160();
        in_v1 = (uint)(iVar1 * 0xc) >> 0xf;
      } while (*(short *)(&DAT_8005ec84 + in_v1 * 2) < 0);
    } while ((param_1 & 1 << (in_v1 + 0x13 & 0x1f)) == 0);
    if (in_v1 == 0xb) break;
    iVar2 = iVar2 + -1;
    if ((param_1 & 0x40000000) == 0) {
      return in_v1;
    }
  }
  return 0xb;
}

