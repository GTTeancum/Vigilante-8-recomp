// addr: 0x800553f4  name: _padSetMainMode

bool _padSetMainMode(int param_1,char param_2,undefined1 param_3)

{
  int iVar1;
  
  iVar1 = (*DAT_80065278)();
  if (iVar1 == 0) {
    *(undefined1 *)(param_1 + 0x46) = 1;
    *(code **)(param_1 + 0x14) = PADCMD_OBJ_7F8;
    *(code **)(param_1 + 0x18) = PADCMD_OBJ_84C;
    *(char *)(param_1 + 0x51) = param_2;
    *(undefined1 *)(param_1 + 0x52) = param_3;
    *(bool *)(param_1 + 0x53) = param_2 == *(char *)(param_1 + 0xe4);
  }
  return iVar1 == 0;
}

