// addr: 0x80040234  name: FUN_80040234

undefined4 * FUN_80040234(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  puVar1 = (undefined4 *)FUN_8001ac44(DAT_800737d8,0x16,0x80,8);
  *(undefined1 *)(puVar1 + 1) = 4;
  *puVar1 = 0xb4;
  uVar2 = param_1[1];
  uVar3 = param_1[2];
  puVar1[0x12] = *param_1;
  puVar1[0x13] = uVar2;
  puVar1[0x14] = uVar3;
  puVar1[0x19] = &LAB_8004007c;
  FUN_8002036c(puVar1);
  return puVar1;
}

