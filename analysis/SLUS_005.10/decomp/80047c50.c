// addr: 0x80047c50  name: _UDIVMOD_OBJ_41C

uint _UDIVMOD_OBJ_41C(uint param_1,uint param_2,uint param_3)

{
  uint uVar1;
  uint uVar2;
  longlong lVar3;
  uint in_v1;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint in_t1;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint in_t3;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  uint *in_t9;
  uint uVar13;
  
  uVar11 = 0x20 - ((byte)(&DAT_80010c64)[param_1 >> (in_v1 & 0x1f)] + in_v1);
  uVar12 = 0x20 - uVar11;
  if (uVar11 == 0) {
    if ((param_3 < param_2) || (in_t1 <= in_t3)) {
      uVar11 = _UDIVMOD_OBJ_47C(in_t3 - in_t1,(param_2 - param_3) - (uint)(in_t3 < in_t3 - in_t1));
      return uVar11;
    }
    uVar8 = 0;
    if (in_t9 != (uint *)0x0) {
      uVar11 = _UDIVMOD_OBJ_5E4();
      return uVar11;
    }
  }
  else {
    uVar5 = param_3 << (uVar11 & 0x1f) | in_t1 >> (uVar12 & 0x1f);
    uVar7 = in_t1 << (uVar11 & 0x1f);
    uVar8 = param_2 >> (uVar12 & 0x1f);
    uVar4 = param_2 << (uVar11 & 0x1f) | in_t3 >> (uVar12 & 0x1f);
    uVar6 = uVar5 >> 0x10;
    uVar13 = uVar8 / uVar6;
    if (uVar6 == 0) {
      trap(0x1c00);
    }
    uVar1 = uVar13 * (uVar5 & 0xffff);
    uVar8 = uVar8 % uVar6 << 0x10 | uVar4 >> 0x10;
    uVar10 = in_t3 << (uVar11 & 0x1f);
    uVar9 = uVar13;
    if (uVar8 < uVar1) {
      uVar8 = uVar8 + uVar5;
      uVar9 = uVar13 - 1;
      if ((uVar5 <= uVar8) && (uVar8 < uVar1)) {
        uVar9 = uVar13 - 2;
        uVar8 = uVar8 + uVar5;
      }
    }
    uVar13 = (uVar8 - uVar1) / uVar6;
    if (uVar6 == 0) {
      trap(0x1c00);
    }
    uVar2 = uVar13 * (uVar5 & 0xffff);
    uVar4 = (uVar8 - uVar1) % uVar6 << 0x10 | uVar4 & 0xffff;
    uVar8 = uVar13;
    if (uVar4 < uVar2) {
      uVar4 = uVar4 + uVar5;
      uVar8 = uVar13 - 1;
      if ((uVar5 <= uVar4) && (uVar4 < uVar2)) {
        uVar8 = uVar13 - 2;
        uVar4 = uVar4 + uVar5;
      }
    }
    uVar8 = uVar9 << 0x10 | uVar8;
    uVar4 = uVar4 - uVar2;
    lVar3 = (ulonglong)uVar8 * (ulonglong)uVar7;
    uVar6 = (uint)lVar3;
    uVar13 = (uint)((ulonglong)lVar3 >> 0x20);
    uVar7 = uVar6 - uVar7;
    if ((uVar4 < uVar13) || ((uVar13 == uVar4 && (uVar10 < uVar6)))) {
      uVar8 = uVar8 - 1;
      lVar3 = CONCAT44((uVar13 - uVar5) - (uint)(uVar6 < uVar7),uVar7);
    }
    if (in_t9 != (uint *)0x0) {
      uVar7 = uVar10 - (int)lVar3;
      uVar4 = (uVar4 - (int)((ulonglong)lVar3 >> 0x20)) - (uint)(uVar10 < uVar7);
      *in_t9 = uVar4 << (uVar12 & 0x1f) | uVar7 >> (uVar11 & 0x1f);
      in_t9[1] = uVar4 >> (uVar11 & 0x1f);
    }
  }
  return uVar8;
}

