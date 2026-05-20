// addr: 0x800551f4  name: PADCMD_OBJ_560

undefined4 PADCMD_OBJ_560(int param_1,int param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  byte bVar2;
  undefined4 uVar3;
  
  while (param_1 != -1) {
    param_1 = param_1 + -1;
    if (*(char *)(param_2 + 0x48) == '\0') goto PADCMD_OBJ_5CC;
    uVar1 = *param_3;
    param_3 = param_3 + 1;
    *DAT_800a4c88 = uVar1;
    DAT_800a4c88 = DAT_800a4c88 + 1;
    *(char *)(param_2 + 0x48) = *(char *)(param_2 + 0x48) + -1;
  }
  if (*(char *)(param_2 + 0x48) != '\0') {
    return 0;
  }
PADCMD_OBJ_5CC:
  bVar2 = *(char *)(param_2 + 0x47) + 1;
  *(byte *)(param_2 + 0x47) = bVar2;
  if (*(byte *)(param_2 + 0xea) <= bVar2) {
    *(undefined1 *)(param_2 + 0x49) = 6;
    *(undefined1 *)(param_2 + 0x46) = 0xfe;
    uVar3 = PADCMD_OBJ_60C();
    return uVar3;
  }
  *(undefined1 *)(param_2 + 0x48) = 0;
  uVar3 = PADCMD_OBJ_5C4();
  return uVar3;
}

