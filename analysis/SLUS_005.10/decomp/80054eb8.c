// addr: 0x80054eb8  name: _padLoadActInfo

undefined4 _padLoadActInfo(int *param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  if (param_2 != 0) {
    uVar1 = 0;
    if (param_1[1] == 0) {
      iVar2 = (*DAT_80065278)();
      uVar1 = 1;
      if (iVar2 != 0) goto PADCMD_OBJ_26C;
      *(undefined1 *)((int)param_1 + 0x49) = 4;
      *(undefined1 *)((int)param_1 + 0x46) = 1;
      param_1[5] = (int)PADCMD_OBJ_2F8;
      param_1[6] = (int)PADCMD_OBJ_3A0;
      iVar2 = (param_2 + 3 >> 2) * 4;
      *param_1 = iVar2;
      *(undefined1 *)((int)param_1 + 0x47) = 0;
      iVar2 = iVar2 + ((int)(*(byte *)((int)param_1 + 0xe3) + 1) >> 1) * 4;
      param_1[1] = iVar2;
      param_1[2] = iVar2 + ((uint)*(byte *)((int)param_1 + 0xe9) * 5 + 3 & 0xffc);
    }
    return uVar1;
  }
PADCMD_OBJ_26C:
  uVar1 = PADCMD_OBJ_2E4();
  return uVar1;
}

