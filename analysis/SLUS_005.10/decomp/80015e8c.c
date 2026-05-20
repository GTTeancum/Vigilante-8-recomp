// addr: 0x80015e8c  name: FUN_80015e8c

void FUN_80015e8c(void)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined4 uVar9;
  uint unaff_s1;
  undefined1 auStack_810 [40];
  undefined4 uStack_7e8;
  undefined4 uStack_7e4;
  undefined4 uStack_7e0;
  undefined4 uStack_7dc;
  undefined4 uStack_7d8;
  undefined4 uStack_7d4;
  uint uStack_7d0;
  undefined4 uStack_7cc;
  undefined1 auStack_772 [8];
  undefined1 auStack_76a [1882];
  
  CdInit();
  FUN_800154f4(auStack_810,0x10,1);
  DAT_8006f608 = uStack_7e8;
  DAT_8006f60c = uStack_7e4;
  DAT_8006f610 = uStack_7e0;
  DAT_8006f614 = uStack_7dc;
  DAT_8006f618 = uStack_7d8;
  DAT_8006f61c = uStack_7d4;
  DAT_8006f620 = uStack_7d0;
  DAT_8006f624 = uStack_7cc;
  uVar1 = (uint)(auStack_76a + 3) & 3;
  iVar3 = *(int *)(auStack_76a + 3 + -uVar1);
  uVar5 = (uint)auStack_76a & 3;
  uVar7 = *(uint *)(auStack_76a + -uVar5);
  uVar2 = (uint)(auStack_772 + 3) & 3;
  iVar4 = *(int *)(auStack_772 + 3 + -uVar2);
  uVar6 = (uint)auStack_772 & 3;
  uVar8 = *(uint *)(auStack_772 + -uVar6);
  uVar9 = FUN_800116f4(0x514);
  uRam000006b4 = FUN_80015c68(uVar9,(iVar4 << (3 - uVar2) * 8 |
                                    unaff_s1 & 0xffffffffU >> (uVar2 + 1) * 8) &
                                    -1 << (4 - uVar6) * 8 | uVar8 >> uVar6 * 8,
                              (iVar3 << (3 - uVar1) * 8 |
                              uStack_7d0 & 0xffffffffU >> (uVar1 + 1) * 8) & -1 << (4 - uVar5) * 8 |
                              uVar7 >> uVar5 * 8);
  return;
}

