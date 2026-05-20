// addr: 0x800544d0  name: FUN_800544d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_800544d0(int param_1)

{
  code *pcVar1;
  int iVar2;
  dword *pdVar3;
  undefined4 uVar4;
  
  JOY_MCD_CTRL = 0;
  JOY_MCD_MODE = 0xd;
  JOY_MCD_BAUD = 0x88;
  uVar4 = 0x91;
  if (*(char *)(param_1 + 0xe8) == '\b') {
    uVar4 = 0x50;
  }
  FUN_800566b4(uVar4);
  JOY_MCD_CTRL = 0x1003;
  if (DAT_8006529c != 0) {
    JOY_MCD_CTRL = 0x3003;
  }
  iVar2 = *(int *)(&DAT_800652b4 + DAT_8006529c * 4);
  pcVar1 = DAT_8006527c;
  if (-1 < iVar2) {
    while (DAT_8006527c = pcVar1, 0 < iVar2) {
      iVar2 = *(int *)(&DAT_800652b4 + DAT_8006529c * 4) + -1;
      *(int *)(&DAT_800652b4 + DAT_8006529c * 4) = iVar2;
      (*DAT_8006527c)(*(int *)(param_1 + 0xc) + iVar2 * 0xf0);
      pcVar1 = DAT_8006527c;
      iVar2 = *(int *)(&DAT_800652b4 + DAT_8006529c * 4);
    }
    if (*(int *)(&DAT_800652b4 + DAT_8006529c * 4) == 0) {
      *(int *)(&DAT_800652b4 + DAT_8006529c * 4) = -1;
      (*pcVar1)(param_1);
      (*DAT_80065280)(param_1);
    }
  }
  pdVar3 = &JOY_MCD_DATA;
  if (((ushort)JOY_MCD_STAT & 0x200) != 0) {
    JOY_MCD_CTRL = JOY_MCD_CTRL | 0x10;
    if (((ushort)JOY_MCD_STAT & 0x200) != 0) {
      JOY_MCD_DATA._0_1_ = 1;
      FUN_80054c48();
      uVar4 = 0;
      pdVar3 = (dword *)(uint)(byte)JOY_MCD_DATA;
      goto LAB_800546d0;
    }
    _I_STAT = 0xffffff7f;
    pdVar3 = (dword *)&I_STAT;
  }
  if ((*(char *)(param_1 + 0x50) == '\0') || (uVar4 = 0, *(char *)(param_1 + 0x36) == '\0')) {
    pdVar3 = *(dword **)(param_1 + 0x3c);
    uVar4 = 1;
    *(undefined1 *)pdVar3 = 0;
  }
LAB_800546d0:
  return CONCAT44(pdVar3,uVar4);
}

