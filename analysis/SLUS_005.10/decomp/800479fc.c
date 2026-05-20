// addr: 0x800479fc  name: _UDIVMOD_OBJ_1C8

uint _UDIVMOD_OBJ_1C8(uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint in_v1;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int in_t1;
  uint uVar7;
  uint in_t3;
  uint uVar8;
  int in_t9;
  
  uVar8 = 0x20 - ((byte)(&DAT_80010c64)[param_1 >> (in_v1 & 0x1f)] + in_v1);
  if (uVar8 == 0) {
    uVar8 = _UDIVMOD_OBJ_2E4(param_1,param_2 - in_t1);
    return uVar8;
  }
  uVar7 = in_t1 << (uVar8 & 0x1f);
  uVar4 = param_2 >> (0x20 - uVar8 & 0x1f);
  uVar5 = param_2 << (uVar8 & 0x1f) | in_t3 >> (0x20 - uVar8 & 0x1f);
  uVar6 = uVar7 >> 0x10;
  if (uVar6 == 0) {
    trap(0x1c00);
  }
  uVar1 = (uVar4 / uVar6) * (uVar7 & 0xffff);
  uVar4 = uVar4 % uVar6 << 0x10 | uVar5 >> 0x10;
  uVar8 = in_t3 << (uVar8 & 0x1f);
  if (((uVar4 < uVar1) && (uVar4 = uVar4 + uVar7, uVar7 <= uVar4)) && (uVar4 < uVar1)) {
    uVar4 = uVar4 + uVar7;
  }
  if (uVar6 == 0) {
    trap(0x1c00);
  }
  uVar3 = ((uVar4 - uVar1) / uVar6) * (uVar7 & 0xffff);
  uVar4 = (uVar4 - uVar1) % uVar6 << 0x10 | uVar5 & 0xffff;
  if (((uVar4 < uVar3) && (uVar4 = uVar4 + uVar7, uVar7 <= uVar4)) && (uVar4 < uVar3)) {
    uVar4 = uVar4 + uVar7;
  }
  uVar5 = uVar7 >> 0x10;
  uVar6 = (uVar4 - uVar3) / uVar5;
  if (uVar5 == 0) {
    trap(0x1c00);
  }
  uVar1 = uVar6 * (uVar7 & 0xffff);
  uVar3 = (uVar4 - uVar3) % uVar5 << 0x10 | uVar8 >> 0x10;
  uVar4 = uVar6;
  if (uVar3 < uVar1) {
    uVar3 = uVar3 + uVar7;
    uVar4 = uVar6 - 1;
    if ((uVar7 <= uVar3) && (uVar3 < uVar1)) {
      uVar4 = uVar6 - 2;
      uVar3 = uVar3 + uVar7;
    }
  }
  uVar6 = (uVar3 - uVar1) / uVar5;
  if (uVar5 == 0) {
    trap(0x1c00);
  }
  uVar2 = uVar6 * (uVar7 & 0xffff);
  uVar5 = (uVar3 - uVar1) % uVar5 << 0x10 | uVar8 & 0xffff;
  uVar8 = uVar6;
  if (uVar5 < uVar2) {
    uVar5 = uVar5 + uVar7;
    uVar8 = uVar6 - 1;
    if ((uVar7 <= uVar5) && (uVar5 < uVar2)) {
      uVar8 = uVar6 - 2;
    }
  }
  if (in_t9 == 0) {
    return uVar4 << 0x10 | uVar8;
  }
  uVar8 = _UDIVMOD_OBJ_5E4();
  return uVar8;
}

