// addr: 0x80043408  name: FUN_80043408

int * FUN_80043408(undefined4 *param_1,uint *param_2,int *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  gte_ldR11R12(*param_1);
  gte_ldR13R21(param_1[1]);
  gte_ldR22R23(param_1[2]);
  gte_ldR31R32(param_1[3]);
  gte_ldR33(param_1[4]);
  gte_ldsv_((int)*param_2 >> 0xf,(int)param_2[1] >> 0xf,(int)param_2[2] >> 0xf);
  uVar1 = *param_2 & 0x7fff;
  uVar2 = param_2[1] & 0x7fff;
  uVar3 = param_2[2] & 0x7fff;
  gte_rtir_sf0_b();
  iVar4 = param_1[5];
  iVar5 = param_1[6];
  iVar6 = param_1[7];
  gte_ldtr(iVar4,iVar5,iVar6);
  read_mt(iVar4,iVar5,iVar6);
  gte_ldsv_(uVar1,uVar2,uVar3);
  gte_rtirtr_b();
  read_mt(uVar1,uVar2,uVar3);
  *param_3 = uVar1 + iVar4 * 8;
  param_3[1] = uVar2 + iVar5 * 8;
  param_3[2] = uVar3 + iVar6 * 8;
  return param_3;
}

