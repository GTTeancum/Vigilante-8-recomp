// addr: 0x800561d8  name: PADPORTD_OBJ_604

int PADPORTD_OBJ_604(undefined *param_1)

{
  undefined *puVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = 0;
  iVar3 = 0x10;
  puVar1 = &DAT_800a4d28;
  do {
    if (param_1 == puVar1) {
      return iVar3;
    }
    iVar3 = iVar3 + 0x10;
    iVar2 = iVar2 + 1;
    puVar1 = puVar1 + 0xf0;
  } while (iVar2 < 2);
  return 0xff;
}

