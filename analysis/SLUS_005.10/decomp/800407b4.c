// addr: 0x800407b4  name: FUN_800407b4

void FUN_800407b4(uint param_1,undefined2 param_2,uint *param_3)

{
  uint *puVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  
  puVar2 = (uint *)FUN_8001d470(0x9c);
  *puVar2 = *puVar2 | 0xa4;
  uVar3 = param_3[1];
  uVar4 = param_3[2];
  puVar2[0x12] = *param_3;
  puVar2[0x13] = uVar3;
  puVar2[0x14] = uVar4;
  puVar2[0x19] = (uint)&LAB_80040540;
  puVar2[0x15] = 0x10000;
  FUN_8001d708(puVar2);
  *(undefined2 *)((int)puVar2 + 0x82) = 7;
  puVar2[0x26] = param_1;
  *(undefined2 *)((int)puVar2 + 0x96) = param_2;
  puVar2[0x21] = 0x100;
  puVar2[0x22] = 0xfffffe00;
  puVar2[0x23] = 0;
  uVar3 = (int)puVar2 + 0x93U & 3;
  puVar1 = (uint *)(((int)puVar2 + 0x93U) - uVar3);
  *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | 0x400000U >> (3 - uVar3) * 8;
  uVar3 = (uint)(puVar2 + 0x24) & 3;
  puVar1 = (uint *)((int)(puVar2 + 0x24) - uVar3);
  *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | 0x400000 << uVar3 * 8;
  *(undefined2 *)(puVar2 + 0x25) = 0x20;
  return;
}

