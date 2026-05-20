// addr: 0x800170c8  name: FUN_800170c8

int FUN_800170c8(uint param_1,int param_2)

{
  int iVar1;
  long lVar2;
  uint uVar3;
  
  gte_ldLZCS(param_2);
  iVar1 = gte_stLZCR();
  uVar3 = 0x23 - iVar1 >> 1;
  if ((int)(uVar3 << 0x1b) < 0) {
    param_1 = param_2 >> (uVar3 * 2 & 0x1f);
  }
  else {
    param_1 = param_1 >> (uVar3 * 2 & 0x1f);
    if (uVar3 << 0x1b != 0) {
      param_1 = param_1 | param_2 << (uVar3 * -2 & 0x1f);
    }
  }
  lVar2 = SquareRoot0(param_1);
  return lVar2 << (uVar3 & 0x1f);
}

