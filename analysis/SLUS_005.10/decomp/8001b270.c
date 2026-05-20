// addr: 0x8001b270  name: FUN_8001b270

int FUN_8001b270(int param_1)

{
  ushort *puVar1;
  int iVar2;
  
  puVar1 = (ushort *)FUN_8001b1f8();
  if (puVar1 == (ushort *)0x0) {
    iVar2 = 0;
  }
  else {
    iVar2 = *(int *)((*puVar1 & 0xfff) * 4 + *(int *)(**(int **)(param_1 + 0x58) + 4));
    iVar2 = FUN_8001b3d4(*(int **)(param_1 + 0x58),
                         *(short *)(iVar2 + 0x12) +
                         (*(ushort *)(*(int *)(iVar2 + 0x14) + 0x12) & 0x3fff));
    *(ushort *)(iVar2 + 8) = *(ushort *)(iVar2 + 8) | 0x20;
  }
  return iVar2;
}

