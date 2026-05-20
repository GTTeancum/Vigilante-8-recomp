// addr: 0x800432d0  name: FUN_800432d0

int * FUN_800432d0(uint *param_1,int *param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  iVar4 = (int)*param_1 >> 0xf;
  iVar5 = (int)param_1[1] >> 0xf;
  iVar6 = (int)param_1[2] >> 0xf;
  gte_ldsv_(iVar4,iVar5,iVar6);
  uVar1 = *param_1 & 0x7fff;
  uVar2 = param_1[1] & 0x7fff;
  gte_rtir_sf0_b();
  uVar3 = param_1[2] & 0x7fff;
  read_mt(iVar4,iVar5,iVar6);
  gte_ldsv_(uVar1,uVar2,uVar3);
  gte_rtirtr_b();
  read_mt(uVar1,uVar2,uVar3);
  *param_2 = uVar1 + iVar4 * 8;
  param_2[1] = uVar2 + iVar5 * 8;
  param_2[2] = uVar3 + iVar6 * 8;
  return param_2;
}

