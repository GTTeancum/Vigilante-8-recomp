// addr: 0x801003f4  name: FUN_801003f4

void FUN_801003f4(int param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 0x48);
  if (iVar2 != 0) {
    iVar3 = (int)*(char *)(param_1 + 5);
    if (iVar3 != 0) {
      FUN_80044574/*0x80044574*/(iVar3,iVar2);
    }
    cVar1 = FUN_8004410c/*0x8004410c*/(iVar3 != 0);
    *(char *)(param_1 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),param_2,iVar2);
  }
  if (*(char *)(param_1 + 5) != '\0') {
    FUN_800441c8/*0x800441c8*/();
    *(undefined1 *)(param_1 + 5) = 0;
  }
  return;
}

