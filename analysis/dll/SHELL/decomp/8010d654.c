// addr: 0x8010d654  name: FUN_8010d654

undefined4 FUN_8010d654(int param_1)

{
  undefined2 uVar1;
  int in_v0;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_14;
  
  iVar3 = 200000;
  do {
    iVar3 = iVar3 + -1;
    if (in_v0 == 0) break;
  } while (iVar3 != 0);
  uVar2 = 0;
  if (*(uint *)(param_1 + 0x48) <= *(uint *)(local_14 + 8)) {
    iVar3 = local_14;
    if (*(int *)(param_1 + 0x4c) != 0) {
      iVar3 = 1;
    }
    uVar1 = *(undefined2 *)(iVar3 + 0x10);
    *(undefined2 *)(param_1 + 0x34) = uVar1;
    *(undefined2 *)(param_1 + 0x2c) = uVar1;
    uVar1 = *(undefined2 *)(local_14 + 0x12);
    *(undefined2 *)(param_1 + 0x42) = uVar1;
    *(undefined2 *)(param_1 + 0x36) = uVar1;
    *(undefined2 *)(param_1 + 0x2e) = uVar1;
    uVar2 = *(undefined4 *)(local_14 + 8);
    *(int *)(param_1 + 0x18) = 1 - *(int *)(param_1 + 0x18);
    *(undefined4 *)(param_1 + 0x48) = uVar2;
    uVar2 = *(undefined4 *)(param_1 + 0x48);
  }
  return uVar2;
}

