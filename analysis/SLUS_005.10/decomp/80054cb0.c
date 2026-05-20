// addr: 0x80054cb0  name: _padSendAtLoadInfo

void _padSendAtLoadInfo(int param_1)

{
  byte bVar1;
  
  bVar1 = *(byte *)(param_1 + 0x46);
  if (bVar1 == 3) {
    PADCMD_OBJ_8E0(param_1,*(undefined1 *)(param_1 + 0xe4));
    PADCMD_OBJ_90();
    return;
  }
  if (bVar1 < 4) {
    if (bVar1 != 2) {
      PADCMD_OBJ_90();
      return;
    }
    PADCMD_OBJ_8CC();
    PADCMD_OBJ_90();
    return;
  }
  if (bVar1 != 4) {
    PADCMD_OBJ_90();
    return;
  }
  PADCMD_OBJ_920(param_1,*(undefined1 *)(param_1 + 0x47));
  return;
}

