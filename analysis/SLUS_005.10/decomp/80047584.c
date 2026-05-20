// addr: 0x80047584  name: S_N2P_OBJ_200

uint S_N2P_OBJ_200(uint param_1,uint param_2)

{
  int iVar1;
  uint in_v1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  uint in_t8;
  
  iVar9 = 1 << (in_v1 & 0x1f);
  uVar5 = 0x1000;
  iVar7 = 0;
  iVar2 = iVar9 * 0x1000;
  do {
    uVar5 = uVar5 * 0x103b >> 0xc;
    iVar3 = 0;
    iVar8 = 0;
    uVar6 = iVar9 * uVar5 - iVar2 >> 5;
    uVar4 = uVar6;
    do {
      if (((uint)(iVar2 + iVar8) >> 0xc <= (param_1 & 0xffff)) &&
         (iVar1 = iVar7 * 0x20 + iVar3, (param_1 & 0xffff) < iVar2 + uVar4 >> 0xc))
      goto S_N2P_OBJ_2A8;
      uVar4 = uVar4 + uVar6;
      iVar3 = iVar3 + 1;
      iVar8 = iVar8 + uVar6;
    } while (iVar3 < 0x20);
    iVar7 = iVar7 + 1;
    iVar2 = iVar9 * uVar5;
  } while (iVar7 < 0x30);
  iVar1 = 0x600;
S_N2P_OBJ_2A8:
  iVar2 = iVar1;
  if (iVar1 < 0) {
    iVar2 = iVar1 + 0x7f;
  }
  return ((in_t8 & 0xffff) + (iVar2 >> 7) + (in_v1 - 0xc) * 0xc) * 0x100 |
         (param_2 & 0xffff) + iVar1 + (iVar2 >> 7) * -0x80;
}

