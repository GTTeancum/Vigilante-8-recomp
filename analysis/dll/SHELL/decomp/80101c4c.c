// addr: 0x80101c4c  name: FUN_80101c4c

void FUN_80101c4c(undefined4 *param_1,int param_2,int *param_3)

{
  uint uVar1;
  undefined1 *puVar2;
  undefined4 local_20;
  undefined4 local_1c;
  
  local_20 = *param_1;
  local_1c = param_1[1];
  FUN_80019e20/*0x80019e20*/();
  FUN_8001a0ac/*0x8001a0ac*/(&local_20,0);
  FUN_80019f9c/*0x80019f9c*/(&local_20,0xffffff);
  local_20._0_2_ = (short)local_20 + 1;
  local_20._2_2_ = local_20._2_2_ + 1;
  local_1c = CONCAT22(local_1c._2_2_ + -2,(short)local_1c + -2);
  FUN_80019f9c/*0x80019f9c*/(&local_20,0xffffff);
  uVar1 = (uint)local_20._2_2_;
  puVar2 = (undefined1 *)(uVar1 + 7);
  local_20._2_2_ = (ushort)puVar2;
  if (puVar2 != (undefined1 *)0x0) {
    *puVar2 = 0;
    FUN_80019a58/*0x80019a58*/(param_3,param_2,&local_20,2);
    param_2 = uVar1 + 8;
    local_20._2_2_ = local_20._2_2_ + *(byte *)(*param_3 + 6);
    param_3 = (int *)0x1;
    *puVar2 = 10;
  }
  FUN_80019a58/*0x80019a58*/(param_3,param_2,&local_20,2);
  return;
}

