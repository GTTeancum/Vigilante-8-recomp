// addr: 0x8004352c  name: FUN_8004352c

int * FUN_8004352c(undefined4 param_1,uint *param_2,int *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  FUN_8004366c();
  iVar4 = (int)*param_2 >> 0xf;
  iVar5 = (int)param_2[1] >> 0xf;
  iVar6 = (int)param_2[2] >> 0xf;
  gte_ldsv_(iVar4,iVar5,iVar6);
  uVar1 = *param_2 & 0x7fff;
  uVar2 = param_2[1] & 0x7fff;
  gte_rtir_sf0_b();
  uVar3 = param_2[2] & 0x7fff;
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

