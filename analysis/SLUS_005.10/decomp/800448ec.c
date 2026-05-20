// addr: 0x800448ec  name: FUN_800448ec

uint FUN_800448ec(int *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = FUN_80016a20();
  uVar2 = 0;
  if (iVar1 < 0x200000) {
    iVar3 = -iVar1 + 0x200000;
    if (iVar3 < 0) {
      iVar3 = -iVar1 + 0x200fff;
    }
    iVar3 = (iVar3 >> 0xc) * (int)sRam000008e4;
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0x1ff;
    }
    if (iVar1 == 0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (*param_1 << 0xc) / iVar1;
    }
    iVar4 = (0x1000 - iVar1) * (iVar3 >> 9);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x1fff;
    }
    iVar1 = (iVar1 + 0x1000) * (iVar3 >> 9);
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1fff;
    }
    uVar2 = iVar4 >> 0xd | (iVar1 >> 0xd) << 0x10;
  }
  return uVar2;
}

