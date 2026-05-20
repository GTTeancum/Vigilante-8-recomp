// addr: 0x80047410  name: _spu_note2pitch

uint _spu_note2pitch(uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  undefined4 local_8;
  
  iVar4 = ((param_3 & 0xffff) * 0x80 + (param_4 & 0xffff)) -
          ((param_1 & 0xffff) * 0x80 + (param_2 & 0xffff));
  iVar3 = iVar4;
  if (iVar4 < 0) {
    iVar3 = -iVar4;
  }
  uVar1 = iVar3 / 0x600;
  if (-1 < iVar4) {
    uVar1 = S_N2P_OBJ_118();
    return uVar1;
  }
  uVar2 = 0;
  if (iVar3 % 0x600 != 0) {
    uVar1 = uVar1 + 1;
    uVar2 = 0x600 - iVar3 % 0x600;
  }
  uVar5 = 0x1000 >> (uVar1 & 0x1f) & 0xffff;
  uVar1 = 0x103b;
  local_8 = uVar5 * 0x103b;
  iVar4 = uVar5 << 0xc;
  iVar3 = 0;
  if ((int)uVar2 < 0) {
    uVar2 = -uVar2;
  }
  if (uVar2 >> 5 != 0) {
    local_8 = uVar5 * 0x103b;
    do {
      iVar4 = local_8;
      uVar1 = uVar1 * 0x103b >> 0xc;
      local_8 = uVar5 * uVar1;
      iVar3 = iVar3 + 1;
    } while (iVar3 < (int)(uVar2 >> 5));
  }
  uVar2 = iVar4 + ((uint)(local_8 - iVar4) >> 5) * (uVar2 & 0x1f) >> 0xc;
  uVar1 = uVar2 & 0xffff;
  if (0x3fff < uVar2) {
    uVar1 = 0x3fff;
  }
  return uVar1;
}

