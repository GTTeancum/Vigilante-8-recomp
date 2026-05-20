// addr: 0x80021460  name: FUN_80021460

void FUN_80021460(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  SVECTOR local_20;
  SVECTOR local_18;
  
  local_20.vx = (short)(-iRam000006d8 / 2);
  local_20.vy = 0;
  local_20.vz = sRam000006d4;
  local_18.vx = (short)(iRam000006d8 / 2);
  local_18.vy = 0;
  local_18.vz = sRam000006d4;
  VectorNormalSS(&local_20,&local_20);
  VectorNormalSS(&local_18,&local_18);
  SetRotMatrix((MATRIX *)&DAT_8006f6e0);
  FUN_8004316c(&local_20,&local_20);
  FUN_8004316c(&local_18,&local_18);
  iVar4 = (int)local_18.vx;
  iVar5 = (int)local_20.vx;
  iVar2 = iVar4;
  if (iVar5 < iVar4) {
    iVar2 = iVar5;
  }
  iVar1 = 0;
  if (iVar2 < 0) {
    iVar1 = iVar2;
  }
  if (iVar4 < iVar5) {
    iVar4 = iVar5;
  }
  iVar2 = 0;
  if (0 < iVar4) {
    iVar2 = iVar4;
  }
  iVar4 = (int)local_18.vz;
  iVar6 = (int)local_20.vz;
  iVar5 = iVar4;
  if (iVar6 < iVar4) {
    iVar5 = iVar6;
  }
  iVar3 = 0;
  if (iVar5 < 0) {
    iVar3 = iVar5;
  }
  if (iVar4 < iVar6) {
    iVar4 = iVar6;
  }
  iVar5 = 0;
  if (0 < iVar4) {
    iVar5 = iVar4;
  }
  FUN_800209cc(uRam000006fc,DAT_8006f6f4 + iVar1 * 0x400,DAT_8006f6f4 + iVar2 * 0x400,
               DAT_8006f6fc + iVar3 * 0x400,DAT_8006f6fc + iVar5 * 0x400);
  return;
}

