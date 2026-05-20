// addr: 0x80100688  name: FUN_80100688

void FUN_80100688(uint *param_1)

{
  uint uVar1;
  int iVar2;
  undefined1 auStack_30 [32];
  
  uVar1 = FUN_80042cdc/*0x80042cdc*/(param_1 + 0x12,0xffffffff);
  param_1[0x29] = uVar1;
  param_1[0x2b] = 1;
  iVar2 = FUN_80042724/*0x80042724*/(uVar1,param_1 + 0x12,auStack_30);
  param_1[0x2c] = iVar2 << 0x10;
  param_1[0x2a] = 0xee6;
  param_1[0x1e] = 0;
  param_1[0x1d] = 0;
  param_1[0x24] = 0;
  *param_1 = *param_1 | 0x180;
  param_1[0x25] = 0;
  param_1[0x26] = 0;
  *(undefined2 *)(param_1 + 0x27) = 0x10;
  *(undefined2 *)((int)param_1 + 0x9e) = 0x20;
  *(undefined2 *)(param_1 + 0x28) = 0x40;
  return;
}

