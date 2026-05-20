// addr: 0x80047b18  name: _UDIVMOD_OBJ_2E4

uint _UDIVMOD_OBJ_2E4(undefined4 param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint in_t1;
  uint in_t3;
  int in_t9;
  uint uVar6;
  
  uVar4 = in_t1 >> 0x10;
  uVar6 = param_2 / uVar4;
  if (uVar4 == 0) {
    trap(0x1c00);
  }
  uVar3 = uVar6 * (in_t1 & 0xffff);
  uVar2 = param_2 % uVar4 << 0x10 | in_t3 >> 0x10;
  uVar5 = uVar6;
  if (uVar2 < uVar3) {
    uVar2 = uVar2 + in_t1;
    uVar5 = uVar6 - 1;
    if ((in_t1 <= uVar2) && (uVar2 < uVar3)) {
      uVar5 = uVar6 - 2;
      uVar2 = uVar2 + in_t1;
    }
  }
  uVar6 = (uVar2 - uVar3) / uVar4;
  if (uVar4 == 0) {
    trap(0x1c00);
  }
  uVar1 = uVar6 * (in_t1 & 0xffff);
  uVar3 = (uVar2 - uVar3) % uVar4 << 0x10 | in_t3 & 0xffff;
  uVar4 = uVar6;
  if (uVar3 < uVar1) {
    uVar3 = uVar3 + in_t1;
    uVar4 = uVar6 - 1;
    if ((in_t1 <= uVar3) && (uVar3 < uVar1)) {
      uVar4 = uVar6 - 2;
    }
  }
  if (in_t9 == 0) {
    return uVar5 << 0x10 | uVar4;
  }
  uVar4 = _UDIVMOD_OBJ_5E4();
  return uVar4;
}

