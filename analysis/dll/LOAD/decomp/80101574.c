// addr: 0x80101574  name: FUN_80101574

void FUN_80101574(int param_1,undefined4 param_2)

{
  undefined1 auStack_30 [32];
  
  do {
    CompMatrixLV/*0x8004cf04*/(param_2,param_1 + 0x10,auStack_30);
    GTE_LoadTransform/*0x80043974*/(auStack_30);
    if ((*(ushort **)(param_1 + 0x30) != (ushort *)0x0) &&
       ((**(ushort **)(param_1 + 0x30) & 1) != 0)) {
      **(ushort **)(param_1 + 0x30) = **(ushort **)(param_1 + 0x30) & 0xfffe | 4;
    }
    if ((*(ushort **)(param_1 + 0x68) != (ushort *)0x0) &&
       ((**(ushort **)(param_1 + 0x68) & 1) != 0)) {
      **(ushort **)(param_1 + 0x68) = **(ushort **)(param_1 + 0x68) & 0xfffe | 4;
    }
    param_1 = *(int *)(param_1 + 0x34);
  } while (param_1 != 0);
  return;
}

