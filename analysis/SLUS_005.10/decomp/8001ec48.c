// addr: 0x8001ec48  name: FUN_8001ec48

uint FUN_8001ec48(uint *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = 0;
  for (uVar2 = param_1[0xe]; uVar2 != 0; uVar2 = *(uint *)(uVar2 + 0x34)) {
    uVar1 = FUN_8001ec48(uVar2);
    uVar3 = uVar3 | uVar1;
  }
  if (uVar3 != 0) {
    *param_1 = *param_1 | 0x800;
  }
  return uVar3 | param_1[0x17] != 0;
}

