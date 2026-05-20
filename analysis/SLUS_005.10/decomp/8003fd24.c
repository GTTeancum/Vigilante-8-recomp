// addr: 0x8003fd24  name: FUN_8003fd24

undefined4 * FUN_8003fd24(undefined4 *param_1,undefined2 param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  puVar1 = (undefined4 *)FUN_8001ac44(DAT_800737d8,param_2,0x80,8);
  *(undefined1 *)(puVar1 + 1) = 1;
  *puVar1 = 0x34;
  uVar3 = param_1[1];
  uVar4 = param_1[2];
  puVar1[0x12] = *param_1;
  puVar1[0x13] = uVar3;
  puVar1[0x14] = uVar4;
  iVar2 = puVar1[0xe];
  puVar1[0x19] = &LAB_8003e80c;
  for (; iVar2 != 0; iVar2 = *(int *)(iVar2 + 0x34)) {
    *(undefined1 **)(iVar2 + 100) = &LAB_8003e7b4;
  }
  FUN_8002036c(puVar1);
  return puVar1;
}

