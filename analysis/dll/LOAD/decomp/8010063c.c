// addr: 0x8010063c  name: FUN_8010063c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010063c(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined2 uVar3;
  short sVar4;
  undefined4 *puVar5;
  undefined4 local_res0 [4];
  
  local_res0[0] = param_1;
  puVar5 = (undefined4 *)Heap_AllocOrRetry/*0x800116f4*/(0x14);
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)(puVar5 + 3) = uVar3;
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)((int)puVar5 + 0xe) = uVar3;
  sVar4 = func_0x800224b4(local_res0);
  *(short *)(puVar5 + 4) = (sVar4 - *(short *)(puVar5 + 3)) + 1;
  sVar4 = func_0x800224b4(local_res0);
  *(short *)((int)puVar5 + 0x12) = (sVar4 - *(short *)((int)puVar5 + 0xe)) + 1;
  func_0x800224b4(local_res0);
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)(puVar5 + 2) = uVar3;
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)((int)puVar5 + 10) = uVar3;
  puVar1 = _DAT_80065aa8;
  puVar2 = puVar5;
  *_DAT_80065aa8 = puVar5;
  _DAT_80065aa8 = puVar2;
  puVar5[1] = puVar1;
  *puVar5 = 0x80065aa4;
  return;
}

