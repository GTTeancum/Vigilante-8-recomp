// addr: 0x800435c0  name: FUN_800435c0

int * FUN_800435c0(int param_1,int *param_2,int *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  FUN_8004366c();
  uVar1 = *param_2 - *(int *)(param_1 + 0x14);
  uVar2 = param_2[1] - *(int *)(param_1 + 0x18);
  uVar3 = param_2[2] - *(int *)(param_1 + 0x1c);
  iVar4 = (int)uVar1 >> 0xf;
  iVar5 = (int)uVar2 >> 0xf;
  iVar6 = (int)uVar3 >> 0xf;
  gte_ldsv_(iVar4,iVar5,iVar6);
  uVar1 = uVar1 & 0x7fff;
  uVar2 = uVar2 & 0x7fff;
  gte_rtir_sf0_b();
  uVar3 = uVar3 & 0x7fff;
  read_mt(iVar4,iVar5,iVar6);
  gte_ldsv_(uVar1,uVar2,uVar3);
  gte_rtir_b();
  read_mt(uVar1,uVar2,uVar3);
  *param_3 = uVar1 + iVar4 * 8;
  param_3[1] = uVar2 + iVar5 * 8;
  param_3[2] = uVar3 + iVar6 * 8;
                    /* WARNING: Treating indirect jump as return */
  return param_3;
}

