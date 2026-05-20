// addr: 0x80016a20  name: FUN_80016a20

int FUN_80016a20(undefined4 param_1)

{
  long lVar1;
  int iVar2;
  int iVar3;
  uint a;
  uint uVar4;
  undefined8 uVar5;
  
  uVar5 = FUN_800171d4(param_1,param_1);
  iVar2 = (int)((ulonglong)uVar5 >> 0x20);
  gte_ldLZCS(iVar2);
  iVar3 = gte_stLZCR();
  uVar4 = 0x23 - iVar3 >> 1;
  if ((int)(uVar4 << 0x1b) < 0) {
    a = iVar2 >> (uVar4 * 2 & 0x1f);
  }
  else {
    a = (uint)uVar5 >> (uVar4 * 2 & 0x1f);
    if (uVar4 << 0x1b != 0) {
      a = a | iVar2 << (uVar4 * -2 & 0x1f);
    }
  }
  lVar1 = SquareRoot0(a);
  return lVar1 << (uVar4 & 0x1f);
}

