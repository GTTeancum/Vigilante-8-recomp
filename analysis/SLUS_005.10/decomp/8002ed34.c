// addr: 0x8002ed34  name: FUN_8002ed34

uint * FUN_8002ed34(uint *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint *puVar11;
  uint *puVar12;
  uint *puVar13;
  uint *puVar14;
  
  puVar13 = (uint *)0x0;
  puVar11 = (uint *)0x0;
  uVar9 = 0xffffffff;
  uVar7 = 0xffffffff;
  if (param_2 == 0) {
    uVar4 = 0;
  }
  else {
    uVar4 = FUN_80016aac(param_1 + 9,param_2 + 0x48);
  }
  piVar3 = (int *)*piRam00000714;
  piVar2 = piRam00000714;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    puVar6 = (uint *)piVar2[2];
    if ((((puVar6 != param_1) && ((char)puVar6[1] != '\x03')) && ((*puVar6 & 0x4000) != 0)) &&
       ((0 < *(short *)((int)puVar6 + 6) || (cRam00000015 == '\x03')))) {
      uVar5 = FUN_80016aac(param_1 + 9,puVar6 + 0x12);
      if (((uVar4 < uVar5) &&
          (uVar8 = uVar5, uVar10 = uVar9, puVar12 = puVar6, puVar14 = puVar13, uVar5 < uVar7)) ||
         (uVar8 = uVar7, uVar10 = uVar5, puVar12 = puVar11, puVar14 = puVar6, uVar5 < uVar9)) {
        uVar7 = uVar8;
        uVar9 = uVar10;
        puVar11 = puVar12;
        puVar13 = puVar14;
      }
    }
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  if (puVar11 == (uint *)0x0) {
    puVar11 = puVar13;
  }
  return puVar11;
}

