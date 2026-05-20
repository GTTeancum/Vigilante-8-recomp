// addr: 0x80034b5c  name: FUN_80034b5c

undefined4 FUN_80034b5c(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined1 auStack_18 [8];
  
  *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + *(int *)(param_1 + 0x88);
  *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x8c);
  *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + *(int *)(param_1 + 0x90);
  *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - (*(int *)(param_1 + 0x88) >> 4);
  *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) - (*(int *)(param_1 + 0x8c) >> 4);
  *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) - (*(int *)(param_1 + 0x90) >> 4);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x48);
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_1 + 0x4c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x50);
  *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) + 0x38;
  iVar1 = FUN_8001d748(param_1,param_1 + 0x48,auStack_18,0);
  uVar2 = 0;
  if (iVar1 < *(int *)(param_1 + 0x4c)) {
    uVar3 = FUN_800446dc(param_1 + 0x48);
    *(int *)(param_1 + 0x4c) = iVar1;
    uVar2 = FUN_8004410c();
    FUN_800443c8(uVar2,uRam000005f8,0x32,
                 uVar3 >> ((int)*(short *)(param_1 + 0x94) & 0x1fU) &
                 (0x40004000 >> ((int)*(short *)(param_1 + 0x94) & 0x1fU)) - 0x10001U);
    *(short *)(param_1 + 0x94) = *(short *)(param_1 + 0x94) + 1;
    if (*(int *)(param_1 + 0x8c) < 0x2fa) {
      uVar2 = 1;
    }
    else {
      FUN_80016fa8(param_1 + 0x10,auStack_18);
      *(int *)(param_1 + 0x8c) = -*(int *)(param_1 + 0x8c) / 2;
      *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) >> 2;
      *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) >> 2;
      uVar2 = 0;
    }
  }
  return uVar2;
}

