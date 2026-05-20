// addr: 0x80047548  name: _spu_pitch2note

uint _spu_pitch2note(uint param_1,uint param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  
  uVar6 = 0xf;
  uVar1 = (int)(~param_3 & 0xffff) >> 0xf;
  do {
    if ((uVar1 & 1) == 0) {
      uVar1 = S_N2P_OBJ_200();
      return uVar1;
    }
    uVar6 = uVar6 - 1;
    uVar1 = (int)(~param_3 & 0xffff) >> (uVar6 & 0x1f);
  } while (-1 < (int)uVar6);
  iVar7 = 0;
  uVar1 = 0x1000;
  do {
    uVar4 = uVar1 * 0x103b >> 0xc;
    iVar3 = 0;
    iVar8 = 0;
    uVar5 = uVar4 - uVar1 >> 5;
    uVar6 = uVar5;
    do {
      if ((uVar1 + iVar8 >> 0xc <= (param_3 & 0xffff)) &&
         (iVar2 = iVar7 * 0x20 + iVar3, (param_3 & 0xffff) < uVar1 + uVar6 >> 0xc))
      goto S_N2P_OBJ_2A8;
      uVar6 = uVar6 + uVar5;
      iVar3 = iVar3 + 1;
      iVar8 = iVar8 + uVar5;
    } while (iVar3 < 0x20);
    iVar7 = iVar7 + 1;
    uVar1 = uVar4;
  } while (iVar7 < 0x30);
  iVar2 = 0x600;
S_N2P_OBJ_2A8:
  iVar7 = iVar2;
  if (iVar2 < 0) {
    iVar7 = iVar2 + 0x7f;
  }
  return ((param_1 & 0xffff) + (iVar7 >> 7) + -0x90) * 0x100 |
         (param_2 & 0xffff) + iVar2 + (iVar7 >> 7) * -0x80;
}

