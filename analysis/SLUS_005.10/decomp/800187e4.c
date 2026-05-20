// addr: 0x800187e4  name: FUN_800187e4

void FUN_800187e4(undefined4 param_1,undefined2 *param_2)

{
  undefined2 uVar1;
  uint *puVar2;
  
  puVar2 = (uint *)FUN_80018618(param_1,param_2 + 4,param_2 + 5,param_2 + 3);
  param_2[1] = (short)((int)*(short *)(puVar2[3] + 4) << (2 - (*puVar2 & 3) & 0x1f));
  uVar1 = *(undefined2 *)(puVar2[3] + 6);
  *param_2 = 1;
  param_2[2] = uVar1;
  return;
}

