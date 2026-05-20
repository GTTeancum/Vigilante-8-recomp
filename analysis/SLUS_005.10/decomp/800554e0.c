// addr: 0x800554e0  name: PADCMD_OBJ_84C

undefined4 PADCMD_OBJ_84C(int param_1)

{
  undefined4 uVar1;
  
  if (*(char *)(param_1 + 0x53) == '\0') {
    (*DAT_80065260)();
    return 0;
  }
  if (*(char *)(param_1 + 0x46) == '\x02') {
    uVar1 = PADCMD_OBJ_89C();
    return uVar1;
  }
  *(undefined1 *)(param_1 + 0x46) = 0xfe;
  uVar1 = PADCMD_OBJ_898();
  return uVar1;
}

