// addr: 0x80055ea0  name: PADPORTD_OBJ_2CC

undefined4 PADPORTD_OBJ_2CC(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  iVar2 = *(byte *)(param_1 + 0x45) - 3;
  if (*(char *)(param_1 + 0x36) == '\0') {
    if (((5 < iVar2) || (uVar1 = 0, *(char *)(param_1 + iVar2 + 0x57) != '\0')) &&
       (uVar1 = 0, iVar2 < (int)(uint)*(byte *)(param_1 + 0x34))) {
      uVar1 = PADPORTD_OBJ_380();
      return uVar1;
    }
  }
  else {
    if (*(char *)(param_1 + 0x36) != 'M') {
      uVar1 = PADPORTD_OBJ_35C();
      return uVar1;
    }
    uVar1 = 0xff;
    if (iVar2 < (int)(uint)*(byte *)(param_1 + 0x35)) {
      uVar1 = PADPORTD_OBJ_330();
      return uVar1;
    }
  }
  return uVar1;
}

