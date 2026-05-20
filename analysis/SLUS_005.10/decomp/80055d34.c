// addr: 0x80055d34  name: PADPORTD_OBJ_160

void PADPORTD_OBJ_160(undefined4 *param_1)

{
  int iVar1;
  undefined1 *puVar2;
  
  if (*(char *)((int)param_1 + 0x49) != '\0') {
    puVar2 = (undefined1 *)((int)param_1 + 0x5d);
    iVar1 = 5;
    *(undefined1 *)((int)param_1 + 0x49) = 0;
    *(undefined1 *)((int)param_1 + 0x46) = 0;
    *(undefined2 *)((int)param_1 + 0xe6) = 0;
    param_1[5] = 0;
    param_1[6] = 0;
    *(undefined1 *)((int)param_1 + 0xe3) = 0;
    *(undefined1 *)(param_1 + 0x39) = 0;
    *(undefined2 *)((int)param_1 + 0xe6) = 0;
    *(undefined1 *)((int)param_1 + 0xe9) = 0;
    *(undefined1 *)((int)param_1 + 0xea) = 0;
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;
    do {
      *puVar2 = 0xff;
      iVar1 = iVar1 + -1;
      puVar2 = puVar2 + 1;
    } while (-1 < iVar1);
  }
  return;
}

