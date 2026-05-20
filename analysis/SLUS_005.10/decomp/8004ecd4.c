// addr: 0x8004ecd4  name: ratan2

long ratan2(long y,long x)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  long lVar4;
  
  bVar1 = x < 0;
  if (bVar1) {
    x = -x;
  }
  bVar2 = y < 0;
  if (bVar2) {
    y = -y;
  }
  if ((x != 0) || (iVar3 = 0, y != 0)) {
    if (y < x) {
      iVar3 = x >> 10;
      if ((y & 0x7fe00000U) == 0) {
        if (x == 0) {
          trap(0x1c00);
        }
        if ((x == 0xffffffff) && ((y & 0x3fffffU) == 0x200000)) {
          trap(0x1800);
        }
        lVar4 = RATAN_OBJ_150();
        return lVar4;
      }
      if (iVar3 == 0) {
        trap(0x1c00);
      }
      if ((iVar3 == -1) && (y == 0x80000000)) {
        trap(0x1800);
      }
      lVar4 = RATAN_OBJ_B4(y / iVar3);
      return lVar4;
    }
    iVar3 = y >> 10;
    if ((x & 0x7fe00000U) != 0) {
      if (iVar3 == 0) {
        trap(0x1c00);
      }
      if ((iVar3 == -1) && (x == 0x80000000)) {
        trap(0x1800);
      }
      lVar4 = RATAN_OBJ_13C(x / iVar3);
      return lVar4;
    }
    if (y == 0) {
      trap(0x1c00);
    }
    if ((y == 0xffffffff) && (x << 10 == -0x80000000)) {
      trap(0x1800);
    }
    iVar3 = 0x400 - *(short *)(&DAT_800647b4 + ((x << 10) / y) * 2);
    if (bVar1) {
      iVar3 = 0x800 - iVar3;
    }
    if (bVar2) {
      iVar3 = -iVar3;
    }
  }
  return iVar3;
}

