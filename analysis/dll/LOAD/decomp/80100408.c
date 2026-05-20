// addr: 0x80100408  name: FUN_80100408

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80100408(undefined4 param_1)

{
  undefined4 uVar1;
  int *piVar2;
  int iVar3;
  
  iVar3 = 0x12;
  if (_DAT_800737e8 != 0) {
    piVar2 = (int *)&DAT_800737e8;
    do {
      piVar2 = piVar2 + 1;
      iVar3 = iVar3 + 1;
    } while (*piVar2 != 0);
  }
  uVar1 = FUN_8002263c/*0x8002263c*/(param_1,1);
  *(undefined4 *)(&DAT_800737a0 + iVar3 * 4) = uVar1;
  return;
}

