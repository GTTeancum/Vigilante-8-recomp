// addr: 0x80047834  name: __udivmoddi4

uint __udivmoddi4(uint param_1,uint param_2,uint param_3,uint param_4,uint *param_5)

{
  uint uVar1;
  uint uVar2;
  longlong lVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  
  if (param_4 == 0) {
    if (param_2 < param_3) {
      if (0xffff < param_3) {
        iVar5 = 0x18;
        if (param_3 < 0x1000000) {
          iVar5 = 0x10;
        }
        uVar4 = 0x20 - ((uint)(byte)(&DAT_80010c64)[param_3 >> iVar5] + iVar5);
        if (uVar4 != 0) {
          param_3 = param_3 << (uVar4 & 0x1f);
          param_2 = param_2 << (uVar4 & 0x1f) | param_1 >> (0x20 - uVar4 & 0x1f);
          param_1 = param_1 << (uVar4 & 0x1f);
        }
        uVar4 = param_3 >> 0x10;
        uVar11 = param_2 / uVar4;
        if (uVar4 == 0) {
          trap(0x1c00);
        }
        uVar7 = uVar11 * (param_3 & 0xffff);
        uVar6 = param_2 % uVar4 << 0x10 | param_1 >> 0x10;
        uVar8 = uVar11;
        if (uVar6 < uVar7) {
          uVar6 = uVar6 + param_3;
          uVar8 = uVar11 - 1;
          if ((param_3 <= uVar6) && (uVar6 < uVar7)) {
            uVar8 = uVar11 - 2;
            uVar6 = uVar6 + param_3;
          }
        }
        uVar11 = (uVar6 - uVar7) / uVar4;
        if (uVar4 == 0) {
          trap(0x1c00);
        }
        uVar9 = uVar11 * (param_3 & 0xffff);
        uVar6 = (uVar6 - uVar7) % uVar4 << 0x10 | param_1 & 0xffff;
        uVar7 = uVar11;
        if (uVar6 < uVar9) {
          uVar6 = uVar6 + param_3;
          uVar7 = uVar11 - 1;
          if ((param_3 <= uVar6) && (uVar6 < uVar9)) {
            uVar7 = uVar11 - 2;
          }
        }
        uVar4 = _UDIVMOD_OBJ_3B4(uVar9,uVar7,uVar4,uVar8);
        return uVar4;
      }
      uVar4 = _UDIVMOD_OBJ_60();
      return uVar4;
    }
    if (param_3 == 0) {
      param_3 = 1 / 0;
      trap(0x1c00);
    }
    if (param_3 < 0x10000) {
      uVar4 = _UDIVMOD_OBJ_1C8();
      return uVar4;
    }
    iVar5 = 0x18;
    if (param_3 < 0x1000000) {
      iVar5 = 0x10;
    }
    uVar4 = 0x20 - ((uint)(byte)(&DAT_80010c64)[param_3 >> iVar5] + iVar5);
    if (uVar4 == 0) {
      uVar4 = _UDIVMOD_OBJ_2E4(param_3,param_2 - param_3);
      return uVar4;
    }
    param_3 = param_3 << (uVar4 & 0x1f);
    uVar11 = param_2 >> (0x20 - uVar4 & 0x1f);
    uVar7 = param_2 << (uVar4 & 0x1f) | param_1 >> (0x20 - uVar4 & 0x1f);
    uVar8 = param_3 >> 0x10;
    if (uVar8 == 0) {
      trap(0x1c00);
    }
    uVar6 = (uVar11 / uVar8) * (param_3 & 0xffff);
    uVar11 = uVar11 % uVar8 << 0x10 | uVar7 >> 0x10;
    param_1 = param_1 << (uVar4 & 0x1f);
    if (((uVar11 < uVar6) && (uVar11 = uVar11 + param_3, param_3 <= uVar11)) && (uVar11 < uVar6)) {
      uVar11 = uVar11 + param_3;
    }
    if (uVar8 == 0) {
      trap(0x1c00);
    }
    uVar4 = ((uVar11 - uVar6) / uVar8) * (param_3 & 0xffff);
    uVar11 = (uVar11 - uVar6) % uVar8 << 0x10 | uVar7 & 0xffff;
    if (((uVar11 < uVar4) && (uVar11 = uVar11 + param_3, param_3 <= uVar11)) && (uVar11 < uVar4)) {
      uVar11 = uVar11 + param_3;
    }
    uVar7 = param_3 >> 0x10;
    uVar8 = (uVar11 - uVar4) / uVar7;
    if (uVar7 == 0) {
      trap(0x1c00);
    }
    uVar6 = uVar8 * (param_3 & 0xffff);
    uVar4 = (uVar11 - uVar4) % uVar7 << 0x10 | param_1 >> 0x10;
    uVar11 = uVar8;
    if (uVar4 < uVar6) {
      uVar4 = uVar4 + param_3;
      uVar11 = uVar8 - 1;
      if ((param_3 <= uVar4) && (uVar4 < uVar6)) {
        uVar11 = uVar8 - 2;
        uVar4 = uVar4 + param_3;
      }
    }
    uVar8 = (uVar4 - uVar6) / uVar7;
    if (uVar7 == 0) {
      trap(0x1c00);
    }
    uVar9 = uVar8 * (param_3 & 0xffff);
    uVar7 = (uVar4 - uVar6) % uVar7 << 0x10 | param_1 & 0xffff;
    uVar4 = uVar8;
    if (uVar7 < uVar9) {
      uVar7 = uVar7 + param_3;
      uVar4 = uVar8 - 1;
      if ((param_3 <= uVar7) && (uVar7 < uVar9)) {
        uVar4 = uVar8 - 2;
      }
    }
    uVar4 = uVar11 << 0x10 | uVar4;
    if (param_5 != (uint *)0x0) {
      uVar4 = _UDIVMOD_OBJ_5E4();
      return uVar4;
    }
  }
  else {
    uVar4 = 0;
    if (param_2 < param_4) {
      if (param_5 != (uint *)0x0) {
        uVar4 = _UDIVMOD_OBJ_5E4();
        return uVar4;
      }
    }
    else {
      if (param_4 < 0x10000) {
        uVar4 = _UDIVMOD_OBJ_41C();
        return uVar4;
      }
      iVar5 = 0x18;
      if (param_4 < 0x1000000) {
        iVar5 = 0x10;
      }
      uVar11 = 0x20 - ((uint)(byte)(&DAT_80010c64)[param_4 >> iVar5] + iVar5);
      uVar7 = 0x20 - uVar11;
      if (uVar11 == 0) {
        if ((param_4 < param_2) || (param_3 <= param_1)) {
          uVar4 = _UDIVMOD_OBJ_47C(param_1 - param_3,
                                   (param_2 - param_4) - (uint)(param_1 < param_1 - param_3));
          return uVar4;
        }
        uVar4 = 0;
        if (param_5 != (uint *)0x0) {
          uVar4 = _UDIVMOD_OBJ_5E4();
          return uVar4;
        }
      }
      else {
        uVar6 = param_4 << (uVar11 & 0x1f) | param_3 >> (uVar7 & 0x1f);
        param_3 = param_3 << (uVar11 & 0x1f);
        uVar4 = param_2 >> (uVar7 & 0x1f);
        uVar8 = param_2 << (uVar11 & 0x1f) | param_1 >> (uVar7 & 0x1f);
        uVar9 = uVar6 >> 0x10;
        uVar12 = uVar4 / uVar9;
        if (uVar9 == 0) {
          trap(0x1c00);
        }
        uVar1 = uVar12 * (uVar6 & 0xffff);
        uVar4 = uVar4 % uVar9 << 0x10 | uVar8 >> 0x10;
        param_1 = param_1 << (uVar11 & 0x1f);
        uVar10 = uVar12;
        if (uVar4 < uVar1) {
          uVar4 = uVar4 + uVar6;
          uVar10 = uVar12 - 1;
          if ((uVar6 <= uVar4) && (uVar4 < uVar1)) {
            uVar10 = uVar12 - 2;
            uVar4 = uVar4 + uVar6;
          }
        }
        uVar12 = (uVar4 - uVar1) / uVar9;
        if (uVar9 == 0) {
          trap(0x1c00);
        }
        uVar2 = uVar12 * (uVar6 & 0xffff);
        uVar8 = (uVar4 - uVar1) % uVar9 << 0x10 | uVar8 & 0xffff;
        uVar4 = uVar12;
        if (uVar8 < uVar2) {
          uVar8 = uVar8 + uVar6;
          uVar4 = uVar12 - 1;
          if ((uVar6 <= uVar8) && (uVar8 < uVar2)) {
            uVar4 = uVar12 - 2;
            uVar8 = uVar8 + uVar6;
          }
        }
        uVar4 = uVar10 << 0x10 | uVar4;
        uVar8 = uVar8 - uVar2;
        lVar3 = (ulonglong)uVar4 * (ulonglong)param_3;
        uVar9 = (uint)lVar3;
        uVar12 = (uint)((ulonglong)lVar3 >> 0x20);
        param_3 = uVar9 - param_3;
        if ((uVar8 < uVar12) || ((uVar12 == uVar8 && (param_1 < uVar9)))) {
          uVar4 = uVar4 - 1;
          lVar3 = CONCAT44((uVar12 - uVar6) - (uint)(uVar9 < param_3),param_3);
        }
        if (param_5 != (uint *)0x0) {
          uVar6 = param_1 - (int)lVar3;
          uVar8 = (uVar8 - (int)((ulonglong)lVar3 >> 0x20)) - (uint)(param_1 < uVar6);
          *param_5 = uVar8 << (uVar7 & 0x1f) | uVar6 >> (uVar11 & 0x1f);
          param_5[1] = uVar8 >> (uVar11 & 0x1f);
        }
      }
    }
  }
  return uVar4;
}

