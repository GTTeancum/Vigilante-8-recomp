// addr: 0x8010047c  name: FUN_8010047c

uint * FUN_8010047c(int param_1)

{
  uint *puVar1;
  undefined4 uVar2;
  undefined1 auStack_30 [32];
  
  puVar1 = (uint *)FUN_8001d470/*0x8001d470*/(0x9c);
  uVar2 = FUN_8001b07c/*0x8001b07c*/(auStack_30,*(undefined4 *)(param_1 + 0x70));
  CompMatrixLV/*0x8004cf04*/(param_1 + 0x10,uVar2,puVar1 + 4);
  puVar1[0x19] = (uint)FUN_801002b0;
  puVar1[0x15] = 0x40000;
  *(undefined1 *)(puVar1 + 1) = 7;
  *(undefined2 *)(puVar1 + 3) = 5;
  puVar1[0x17] = (uint)&DAT_80100064;
  puVar1[0x26] = (uint)&DAT_80101148;
  *(undefined2 *)((int)puVar1 + 0x82) = 3;
  puVar1[0x21] = 0;
  *puVar1 = *puVar1 | 0x184;
  puVar1[0x22] = 0xfffffe00;
  puVar1[0x23] = 0xc00;
  FUN_800202f4/*0x800202f4*/(puVar1);
  return puVar1;
}

