// addr: 0x80100b40  name: FUN_80100b40

uint * FUN_80100b40(uint *param_1,uint param_2,undefined2 param_3,undefined2 param_4,
                   undefined2 param_5,undefined4 param_6)

{
  uint *puVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;
  
  puVar1 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_2,param_3,0x9c,8);
  FUN_80016da8/*0x80016da8*/(puVar1 + 4);
  uVar3 = param_1[1];
  uVar4 = param_1[2];
  puVar1[9] = *param_1;
  puVar1[10] = uVar3;
  puVar1[0xb] = uVar4;
  puVar1[0x19] = (uint)FUN_80100a18;
  puVar1[0x15] = 0x8000;
  puVar1[0x26] = param_2;
  *(undefined2 *)((int)puVar1 + 0x96) = param_4;
  *puVar1 = *puVar1 | 0xa4;
  *(undefined2 *)((int)puVar1 + 0x82) = param_5;
  FUN_800202f4/*0x800202f4*/(puVar1);
  FUN_80020890/*0x80020890*/(puVar1,param_6);
  uVar2 = FUN_8004410c/*0x8004410c*/();
  FUN_800447e8/*0x800447e8*/(uVar2,*(undefined4 *)(param_2 + 8),0,param_1);
  return puVar1;
}

