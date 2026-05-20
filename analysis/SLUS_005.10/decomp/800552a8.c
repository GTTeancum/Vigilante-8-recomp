// addr: 0x800552a8  name: _padSetActAlign

bool _padSetActAlign(int param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = (*DAT_80065278)();
  if (iVar1 == 0) {
    *(undefined1 *)(param_1 + 0x46) = 1;
    *(code **)(param_1 + 0x14) = PADCMD_OBJ_67C;
    *(undefined4 *)(param_1 + 0x20) = param_2;
    *(code **)(param_1 + 0x18) = PADCMD_OBJ_698;
  }
  return iVar1 == 0;
}

