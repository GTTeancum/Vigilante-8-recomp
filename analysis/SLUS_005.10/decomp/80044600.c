// addr: 0x80044600  name: FUN_80044600

uint FUN_80044600(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  iVar1 = FUN_80016a20();
  iVar2 = iVar1 + 0x200000;
  if (iVar2 < 0) {
    iVar2 = iVar1 + 0x200fff;
  }
  iVar2 = ((int)sRam000008e4 << 9) / (iVar2 >> 0xc);
  gte_ldLZCS(iVar1);
  iVar3 = gte_stLZCR();
  uVar4 = 0xc;
  if ((int)(iVar3 - 1U) < 0xc) {
    uVar4 = iVar3 - 1U;
  }
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (*param_1 << (uVar4 & 0x1f)) / (iVar1 >> (0xc - uVar4 & 0x1f));
  }
  iVar3 = (0x1000 - iVar1) * iVar2;
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x1fff;
  }
  iVar2 = (iVar1 + 0x1000) * iVar2;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x1fff;
  }
  return iVar3 >> 0xd | (iVar2 >> 0xd) << 0x10;
}

