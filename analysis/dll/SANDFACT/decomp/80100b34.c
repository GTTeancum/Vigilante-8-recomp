// addr: 0x80100b34  name: FUN_80100b34

undefined4 FUN_80100b34(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  
  if (param_2 != 1) {
    if ((param_2 == 0) || (param_2 != 2)) {
      iVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 0x48);
      if (iVar2 != 0) {
        FUN_80044574/*0x80044574*/((int)*(char *)(param_1 + 5),iVar2);
      }
      FUN_80020778/*0x80020778*/(param_1);
      FUN_80020890/*0x80020890*/(param_1,0x78);
      FUN_800441c8/*0x800441c8*/((int)*(char *)(param_1 + 5));
      *(undefined1 *)(param_1 + 5) = 0;
    }
    iVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 0x48);
    if (iVar2 != 0) {
      if (*(char *)(param_1 + 5) != '\0') {
        Stream_FatalOom/*0x80015368*/(s_PROB__801000d0);
      }
      cVar1 = FUN_8004410c/*0x8004410c*/();
      *(char *)(param_1 + 5) = cVar1;
      FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),0,iVar2);
      FUN_80020744/*0x80020744*/(param_1);
    }
    FUN_80020890/*0x80020890*/(param_1,0x78);
  }
  FUN_80020890/*0x80020890*/(param_1,*(byte *)(param_1 + 9) + 0x78);
  func_0x800223dc(param_1);
  return 0;
}

