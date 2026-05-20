// addr: 0x80016364  name: FUN_80016364

void FUN_80016364(void)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *r0;
  int iVar5;
  int iVar6;
  uint *puVar7;
  undefined1 uVar8;
  undefined1 *puVar9;
  SVECTOR local_40;
  SVECTOR local_38;
  MATRIX MStack_30;
  
  puVar9 = (undefined1 *)0x0;
  puVar7 = (uint *)(iRam000006bc + iRam00000004 * 12000);
  FUN_80044efc(&local_38,0,8);
  local_38.vy = (short)(*(int *)(iRam000006bc + 24000) << 1);
  local_38.vx = *(short *)(iRam000006bc + 24000);
  local_40.vx = *(short *)(iRam000006bc + 24000);
  local_40.vy = local_38.vy;
  local_40.vz = local_38.vz;
  local_40.pad = local_38.pad;
  iVar1 = 0x80 - *(int *)(iRam000006bc + 0x5dc4);
  iVar2 = 0;
  if (0 < iVar1) {
    iVar2 = iVar1;
  }
  PutDrawEnv((DRAWENV *)(&DAT_8006f208 + iRam00000004 * 0x5c));
  PutDispEnv((DISPENV *)(&DAT_8006f5a0 + iRam00000004 * 0x14));
  RotMatrixYXZ_gte(&local_40,&MStack_30);
  SetRotMatrix(&MStack_30);
  iVar1 = -*(int *)(iRam000006bc + 0x5dc4);
  gte_ldtr(iVar1,iVar1,*(int *)(iRam000006bc + 0x5dc4) * 4 + 0x100);
  FUN_80011a10();
  iVar1 = -0x78;
  do {
    uVar4 = 0xffffff60;
    iVar6 = iVar1 << 0x10;
    iVar5 = (iVar1 + 0x10) * 0x10000;
    r0 = puVar7 + 8;
    do {
      uVar3 = uVar4 & 0xffff;
      gte_ldVXY0(uVar3 + iVar6);
      gte_ldVZ0(0);
      uVar4 = uVar4 + 0x10;
      gte_ldVXY1((uVar4 & 0xffff) + iVar6);
      gte_ldVZ1(0);
      gte_ldVXY2(uVar3 + iVar5);
      gte_ldVZ2(0);
      gte_rtpt();
      uVar8 = (undefined1)iVar2;
      *(undefined1 *)(r0 + -7) = uVar8;
      *(undefined1 *)((int)r0 + -0x1b) = uVar8;
      *(undefined1 *)((int)r0 + -0x1a) = uVar8;
      gte_stSXY0();
      gte_stSXY1();
      gte_stSXY2();
      gte_ldVXY0((uVar4 & 0xffff) + iVar5);
      gte_ldVZ0(0);
      gte_rtps();
      gte_stsxy((long *)r0);
      uVar3 = **(uint **)(puVar9 + 0x60c);
      **(uint **)(puVar9 + 0x60c) = (uint)puVar7 & 0xffffff;
      r0 = r0 + 10;
      *puVar7 = (uint)*(byte *)((int)puVar7 + 3) << 0x18 | uVar3;
      puVar7 = puVar7 + 10;
    } while ((int)uVar4 < 0xa0);
    iVar1 = iVar1 + 0x10;
  } while (iVar1 < 0x78);
  DrawOTag(*(u_long **)(puVar9 + 0x60c));
  iVar1 = *(int *)(puVar9 + 0x6bc);
  iVar2 = *(int *)(iVar1 + 0x5dc4) + 1;
  *(int *)(iVar1 + 0x5dc4) = iVar2;
  *(int *)(iVar1 + 24000) = *(int *)(iVar1 + 24000) + 0x22;
  *(uint *)(iVar1 + 0x5dcc) = iVar2 < 0x80 ^ 1;
  if (*(code **)(iVar1 + 0x5dd0) != (code *)0x0) {
    (**(code **)(iVar1 + 0x5dd0))();
  }
  return;
}

