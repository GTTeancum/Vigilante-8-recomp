// addr: 0x8004c774  name: InvSquareRoot

/* WARNING: Instruction at (ram,0x8004c7c4) overlaps instruction at (ram,0x8004c7c0)
    */

void InvSquareRoot(long a,long *b,long *c)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  
  gte_ldLZCS(a);
  uVar2 = gte_stLZCR();
  if ((uVar2 != 0x20) && (uVar2 != 0)) {
    uVar2 = uVar2 & 0xfffffffe;
    if ((int)(uVar2 - 0x18) < 0) {
      iVar3 = a >> (0x18 - uVar2 & 0x1f);
    }
    else {
      iVar3 = a << (uVar2 - 0x18 & 0x1f);
    }
    sVar1 = *(short *)(&DAT_80060628 + (iVar3 + -0x40) * 2);
    *c = (int)(0x1f - uVar2) >> 1;
    *b = (int)sVar1;
    return;
  }
  return;
}

