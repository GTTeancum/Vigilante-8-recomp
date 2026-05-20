// addr: 0x80041da0  name: FUN_80041da0

void FUN_80041da0(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = 0;
  iVar4 = param_1;
  if (0 < *(short *)(param_1 + 0x12)) {
    do {
      iVar2 = *(int *)(iVar4 + 0x1c);
      uVar1 = (uint)(*(int *)(iVar2 + 4) == param_1);
      if (*(int *)(iVar2 + (1 - uVar1) * 4) == 0) {
        FUN_80045088();
      }
      else {
        *(undefined4 *)(iVar2 + uVar1 * 4) = 0;
      }
      iVar3 = iVar3 + 1;
      iVar4 = iVar4 + 4;
    } while (iVar3 < *(short *)(param_1 + 0x12));
  }
  if (*(int *)(param_1 + 0x18) != 0) {
    FUN_80045088(*(undefined4 *)(*(int *)(param_1 + 0x18) + 8));
    FUN_8001bddc(*(undefined4 *)(param_1 + 0x18));
  }
  FUN_80045088(param_1);
  return;
}

