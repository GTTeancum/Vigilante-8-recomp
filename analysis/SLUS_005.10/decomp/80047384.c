// addr: 0x80047384  name: _spu_2pitch

uint _spu_2pitch(int param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_8;
  
  uVar1 = 0x103b;
  local_8 = param_1 * 0x103b;
  iVar3 = param_1 << 0xc;
  iVar2 = 0;
  if (param_2 >> 5 != 0) {
    local_8 = param_1 * 0x103b;
    do {
      iVar3 = local_8;
      uVar1 = uVar1 * 0x103b >> 0xc;
      local_8 = param_1 * uVar1;
      iVar2 = iVar2 + 1;
    } while (iVar2 < (int)(param_2 >> 5));
  }
  return iVar3 + ((uint)(local_8 - iVar3) >> 5) * (param_2 & 0x1f) >> 0xc;
}

