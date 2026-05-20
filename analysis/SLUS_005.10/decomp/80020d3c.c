// addr: 0x80020d3c  name: FUN_80020d3c

undefined4 FUN_80020d3c(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  SVECTOR local_48;
  SVECTOR local_40;
  MATRIX MStack_38;
  
  if (iRam000006fc == 0) {
    uVar1 = 0;
  }
  else {
    ReadRotMatrix(&MStack_38);
    local_48.vx = (short)(-iRam000006d8 / 2);
    local_48.vy = 0;
    local_48.vz = sRam000006d4;
    local_40.vx = (short)(iRam000006d8 / 2);
    local_40.vy = 0;
    local_40.vz = sRam000006d4;
    VectorNormalSS(&local_48,&local_48);
    VectorNormalSS(&local_40,&local_40);
    SetRotMatrix((MATRIX *)&DAT_8006f6e0);
    FUN_8004316c(&local_48,&local_48);
    FUN_8004316c(&local_40,&local_40);
    SetRotMatrix(&MStack_38);
    iVar4 = (int)local_40.vx;
    iVar5 = (int)local_48.vx;
    iVar3 = iVar4;
    if (iVar5 < iVar4) {
      iVar3 = iVar5;
    }
    iVar2 = 0;
    if (iVar3 < 0) {
      iVar2 = iVar3;
    }
    local_58 = DAT_8006f6f4 + iVar2 * 0x400;
    if (iVar4 < iVar5) {
      iVar4 = iVar5;
    }
    iVar3 = 0;
    if (0 < iVar4) {
      iVar3 = iVar4;
    }
    iVar5 = (int)local_48.vz;
    iVar4 = (int)local_40.vz;
    local_54 = DAT_8006f6f4 + iVar3 * 0x400;
    iVar3 = iVar4;
    if (iVar5 < iVar4) {
      iVar3 = iVar5;
    }
    iVar2 = 0;
    if (iVar3 < 0) {
      iVar2 = iVar3;
    }
    local_50 = DAT_8006f6fc + iVar2 * 0x400;
    if (iVar4 < iVar5) {
      iVar4 = iVar5;
    }
    iVar3 = 0;
    if (0 < iVar4) {
      iVar3 = iVar4;
    }
    local_4c = DAT_8006f6fc + iVar3 * 0x400;
    iVar4 = FUN_80020bec(iRam000006fc,&local_58);
    if ((iVar4 == 0) && (iVar4 = FUN_80020ad0(&DAT_80065a18,&local_58), iVar4 == 0)) {
      return 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}

