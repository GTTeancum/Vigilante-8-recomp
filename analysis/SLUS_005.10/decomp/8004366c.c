// addr: 0x8004366c  name: FUN_8004366c

undefined8 FUN_8004366c(uint *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar1 = *param_1 & 0xffff;
  uVar3 = param_1[1] & 0xffff;
  gte_ldR11R12(uVar1 | param_1[1] - uVar3);
  uVar2 = param_1[2] & 0xffff;
  gte_ldR31R32(uVar3 | param_1[2] - uVar2);
  uVar3 = param_1[3] & 0xffff;
  uVar1 = *param_1 - uVar1 | uVar3;
  uVar2 = param_1[3] - uVar3 | uVar2;
  gte_ldR13R21(uVar1);
  gte_ldR22R23(uVar2);
  gte_ldR33(param_1[4]);
  return CONCAT44(uVar2,uVar1);
}

