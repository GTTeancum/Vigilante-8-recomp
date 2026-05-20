// addr: 0x800436c8  name: FUN_800436c8

undefined8 FUN_800436c8(uint *param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  
  uVar1 = *param_1;
  uVar2 = param_1[1];
  uVar4 = param_1[2];
  if ((int)uVar1 < 0) {
    uVar1 = -uVar1;
  }
  if ((int)(uVar1 << 0x10) < 0) {
    uVar1 = uVar1 ^ 0xffff;
  }
  gte_ldR11R12(uVar1);
  if ((int)uVar2 < 0) {
    uVar2 = -uVar2;
  }
  uVar1 = param_1[3];
  if ((int)(uVar2 << 0x10) < 0) {
    uVar2 = uVar2 ^ 0xffff;
  }
  gte_ldR13R21(uVar2);
  if ((int)uVar4 < 0) {
    uVar4 = -uVar4;
  }
  iVar3 = (int)(short)param_1[4];
  if ((int)(uVar4 << 0x10) < 0) {
    uVar4 = uVar4 ^ 0xffff;
  }
  gte_ldR22R23(uVar4);
  if ((int)uVar1 < 0) {
    uVar1 = -uVar1;
  }
  if ((int)(uVar1 << 0x10) < 0) {
    uVar1 = uVar1 ^ 0xffff;
  }
  gte_ldR31R32(uVar1);
  if (iVar3 < 0) {
    iVar3 = -iVar3;
  }
  gte_ldR33(iVar3);
  return CONCAT44(iVar3,uVar1);
}

