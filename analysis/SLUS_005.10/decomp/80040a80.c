// addr: 0x80040a80  name: FUN_80040a80

void FUN_80040a80(uint param_1,undefined2 param_2,uint *param_3)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  
  puVar1 = (uint *)FUN_8001d470(0x9c);
  *puVar1 = *puVar1 | 0xa4;
  uVar2 = param_3[1];
  uVar3 = param_3[2];
  puVar1[0x12] = *param_3;
  puVar1[0x13] = uVar2;
  puVar1[0x14] = uVar3;
  puVar1[0x19] = (uint)&LAB_80040894;
  puVar1[0x15] = 0x10000;
  FUN_8001d708(puVar1);
  *(undefined2 *)((int)puVar1 + 0x82) = 4;
  puVar1[0x21] = 0x200;
  puVar1[0x26] = param_1;
  *(undefined2 *)((int)puVar1 + 0x96) = param_2;
  puVar1[0x22] = 0xfffffa00;
  puVar1[0x23] = 0;
  return;
}

