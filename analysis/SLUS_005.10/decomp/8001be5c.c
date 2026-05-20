// addr: 0x8001be5c  name: FUN_8001be5c

void FUN_8001be5c(ushort *param_1,short *param_2)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  uint *puVar9;
  undefined **ppuVar10;
  undefined4 uVar11;
  
  if ((*param_1 & 1) != 0) {
    gte_ldR11R12(DAT_8006f700);
    gte_ldR13R21(DAT_8006f704);
    gte_ldR22R23(DAT_8006f708);
    gte_ldR31R32(DAT_8006f70c);
    gte_ldR33(DAT_8006f710);
    gte_ldsv_((int)*param_2,(int)param_2[3],(int)param_2[6]);
    gte_rtir();
    iVar2 = gte_stIR1();
    iVar3 = gte_stIR2();
    iVar4 = gte_stIR3();
    gte_ldsv_((int)param_2[1],(int)param_2[4],(int)param_2[7]);
    gte_rtir();
    iVar5 = gte_stIR1();
    iVar7 = gte_stIR2();
    iVar8 = gte_stIR3();
    gte_ldsv_((int)param_2[2],(int)param_2[5],(int)param_2[8]);
    gte_rtir();
    gte_ldL11L12(iVar2 + iVar5 * 0x10000);
    gte_ldL31L32(iVar4 + iVar8 * 0x10000);
    iVar2 = gte_stIR1();
    iVar4 = gte_stIR2();
    uVar11 = gte_stIR3();
    gte_ldL13L21(iVar3 * 0x10000 + iVar2);
    gte_ldL22L23(iVar7 + iVar4 * 0x10000);
    gte_ldL33(uVar11);
  }
  gte_ldR11R12(*(undefined4 *)param_2);
  gte_ldR13R21(*(undefined4 *)(param_2 + 2));
  gte_ldR22R23(*(undefined4 *)(param_2 + 4));
  gte_ldR31R32(*(undefined4 *)(param_2 + 6));
  gte_ldR33(*(undefined4 *)(param_2 + 8));
  uVar6 = 0x10 - (int)(short)param_1[0x13];
  gte_ldtr(*(int *)(param_2 + 10) >> (uVar6 & 0x1f),*(int *)(param_2 + 0xc) >> (uVar6 & 0x1f),
           *(int *)(param_2 + 0xe) >> (uVar6 & 0x1f));
  ppuVar10 = &PTR_LAB_8001cd60;
  if ((*param_1 & 4) != 0) {
    ppuVar10 = &PTR_LAB_8001cda0;
  }
  if (*(int *)(param_1 + 0xe) == 0) {
    uVar1 = param_1[1];
    uVar11 = FUN_800116f4();
    *(undefined4 *)(param_1 + 0xe) = uVar11;
    FUN_80044c44(uVar11,*(undefined4 *)(param_1 + 0x10),uVar1);
  }
  puVar9 = *(uint **)(param_1 + 0xc);
  iVar2 = *(int *)(param_1 + 4);
  if (*(int *)(param_1 + 10) != 0) {
    gte_ldv3((SVECTOR *)((puVar9[1] & 0xffff) + iVar2),(SVECTOR *)((puVar9[1] >> 0x10) + iVar2),
             (SVECTOR *)((puVar9[2] & 0xffff) + iVar2));
    gte_rtpt_b();
                    /* WARNING: Could not recover jumptable at 0x8001c0d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)((*puVar9 >> 0x18 & 0x3c) + (int)ppuVar10))();
    return;
  }
  return;
}

