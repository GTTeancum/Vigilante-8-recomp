// addr: 0x8004c544  name: SetFogNearFar

void SetFogNearFar(long a,long b,long h)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = b - a;
  if (99 < iVar2) {
    if (iVar2 == 0) {
      trap(0x1c00);
    }
    if ((iVar2 == -1) && (-a * b == -0x80000000)) {
      trap(0x1800);
    }
    if (iVar2 == 0) {
      trap(0x1c00);
    }
    if ((iVar2 == -1) && (b << 0xc == -0x80000000)) {
      trap(0x1800);
    }
    iVar1 = (-a * b) / iVar2 << 8;
    iVar3 = iVar1 / h;
    if (h == 0) {
      trap(0x1c00);
    }
    if ((h == -1) && (iVar1 == -0x80000000)) {
      trap(0x1800);
    }
    if (iVar3 < -0x8000) {
      iVar3 = -0x8000;
    }
    if (0x7fff < iVar3) {
      iVar3 = 0x7fff;
    }
    SetDQA(iVar3);
    SetDQB((b << 0xc) / iVar2 << 0xc);
  }
  return;
}

