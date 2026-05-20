// addr: 0x80100bd0  name: FUN_80100bd0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80100bd0(undefined4 param_1,undefined2 param_2)

{
  uint uVar1;
  uint *puVar2;
  undefined4 uVar3;
  
  puVar2 = (uint *)FUN_8001d470/*0x8001d470*/(0x9c);
  puVar2[0x19] = (uint)FUN_80100a9c;
  *puVar2 = *puVar2 | 0xa4;
  uVar1 = _DAT_800737d8;
  *(undefined2 *)((int)puVar2 + 0x96) = 0x12;
  *(undefined2 *)((int)puVar2 + 0x82) = 4;
  *(undefined2 *)(puVar2 + 3) = 2;
  puVar2[0x21] = 0x200;
  puVar2[0x26] = uVar1;
  puVar2[0x22] = 0xfffffa00;
  puVar2[0x23] = 0;
  uVar3 = FUN_8001b038/*0x8001b038*/(param_1,param_2);
  FUN_8001b2fc/*0x8001b2fc*/(param_1,uVar3,puVar2);
  FUN_80020744/*0x80020744*/(puVar2);
  return;
}

