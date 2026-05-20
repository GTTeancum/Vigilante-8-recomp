// addr: 0x80101704  name: FUN_80101704

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101704(byte *param_1)

{
  bool bVar1;
  bool bVar2;
  uint uVar3;
  uint uVar4;
  byte *pbVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  
  iVar7 = 0xd;
  uVar9 = 0;
  uVar10 = 0;
  do {
    pbVar5 = param_1 + iVar7;
    iVar7 = iVar7 + -1;
    if (0x19 < *pbVar5) goto LAB_801018cc;
    uVar3 = (uint)*pbVar5;
    uVar6 = uVar9 >> 0x1f;
    uVar8 = uVar9 * 3;
    bVar1 = uVar8 < uVar9;
    uVar4 = uVar9 * 0xd;
    bVar2 = uVar4 < uVar9;
    uVar9 = uVar9 * 0x1a + uVar3;
    uVar10 = (((((uVar10 << 1 | uVar6) + uVar10 + (uint)bVar1) * 4 | uVar8 >> 0x1e) + uVar10 +
              (uint)bVar2) * 2 | uVar4 >> 0x1f) + (uint)(uVar9 < uVar3);
  } while (0 < iVar7);
  uVar8 = (uint)*param_1;
  V8_SeedRng/*0x8001714c*/(0x31415926);
  while (uVar8 = uVar8 - 1, uVar8 != 0xffffffff) {
    FUN_80017160/*0x80017160*/();
  }
  uVar8 = func_0x8001719c();
  uVar4 = func_0x8001719c();
  uVar9 = uVar9 ^ uVar4;
  uVar10 = uVar10 ^ uVar8 >> 3;
  iVar7 = 0;
  do {
    iVar7 = iVar7 + 1;
  } while (iVar7 < 0xd);
  iVar7 = 0xb;
LAB_801018cc:
  do {
    do {
      *(char *)((int)&local_30 + iVar7) = (char)(uVar9 & 0x1f);
    } while ((uVar9 & 0x1f) == 0);
    iVar7 = iVar7 + -1;
    uVar9 = uVar9 >> 5 | uVar10 << 0x1b;
    uVar10 = uVar10 >> 5;
  } while (-1 < iVar7);
  _DAT_80065950 = local_30;
  _DAT_80065954 = local_2c;
  _DAT_80065958 = local_28;
  return 1;
}

