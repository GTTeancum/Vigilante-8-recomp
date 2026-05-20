// addr: 0x80047894  name: _UDIVMOD_OBJ_60

void _UDIVMOD_OBJ_60(uint param_1,uint param_2)

{
  uint uVar1;
  uint in_v1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint in_t1;
  uint in_t3;
  uint uVar5;
  uint uVar6;
  
  uVar5 = 0x20 - ((byte)(&DAT_80010c64)[param_1 >> (in_v1 & 0x1f)] + in_v1);
  if (uVar5 != 0) {
    in_t1 = in_t1 << (uVar5 & 0x1f);
    param_2 = param_2 << (uVar5 & 0x1f) | in_t3 >> (0x20 - uVar5 & 0x1f);
    in_t3 = in_t3 << (uVar5 & 0x1f);
  }
  uVar5 = in_t1 >> 0x10;
  uVar6 = param_2 / uVar5;
  if (uVar5 == 0) {
    trap(0x1c00);
  }
  uVar3 = uVar6 * (in_t1 & 0xffff);
  uVar2 = param_2 % uVar5 << 0x10 | in_t3 >> 0x10;
  uVar4 = uVar6;
  if (uVar2 < uVar3) {
    uVar2 = uVar2 + in_t1;
    uVar4 = uVar6 - 1;
    if ((in_t1 <= uVar2) && (uVar2 < uVar3)) {
      uVar4 = uVar6 - 2;
      uVar2 = uVar2 + in_t1;
    }
  }
  uVar6 = (uVar2 - uVar3) / uVar5;
  if (uVar5 == 0) {
    trap(0x1c00);
  }
  uVar1 = uVar6 * (in_t1 & 0xffff);
  uVar2 = (uVar2 - uVar3) % uVar5 << 0x10 | in_t3 & 0xffff;
  uVar3 = uVar6;
  if (uVar2 < uVar1) {
    uVar2 = uVar2 + in_t1;
    uVar3 = uVar6 - 1;
    if ((in_t1 <= uVar2) && (uVar2 < uVar1)) {
      uVar3 = uVar6 - 2;
    }
  }
  _UDIVMOD_OBJ_3B4(uVar1,uVar3,uVar5,uVar4);
  return;
}

