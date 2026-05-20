// addr: 0x8004749c  name: S_N2P_OBJ_118

uint S_N2P_OBJ_118(void)

{
  uint in_v0;
  uint in_v1;
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined4 local_res0;
  
  uVar3 = in_v0 & 0xffff;
  uVar1 = 0x103b;
  local_res0 = uVar3 * 0x103b;
  iVar4 = uVar3 << 0xc;
  iVar2 = 0;
  if ((int)in_v1 < 0) {
    in_v1 = -in_v1;
  }
  if (in_v1 >> 5 != 0) {
    local_res0 = uVar3 * 0x103b;
    do {
      iVar4 = local_res0;
      uVar1 = uVar1 * 0x103b >> 0xc;
      local_res0 = uVar3 * uVar1;
      iVar2 = iVar2 + 1;
    } while (iVar2 < (int)(in_v1 >> 5));
  }
  uVar3 = iVar4 + ((uint)(local_res0 - iVar4) >> 5) * (in_v1 & 0x1f) >> 0xc;
  uVar1 = uVar3 & 0xffff;
  if (0x3fff < uVar3) {
    uVar1 = 0x3fff;
  }
  return uVar1;
}

