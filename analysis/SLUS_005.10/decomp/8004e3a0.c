// addr: 0x8004e3a0  name: DIVFT3A_OBJ_37C

void DIVFT3A_OBJ_37C(uint *param_1,int param_2)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  int in_t0;
  int in_t1;
  int in_t2;
  uint uVar4;
  uint uVar5;
  
  uVar1 = *(ushort *)(param_2 + 0xe);
  uVar2 = *(ushort *)(in_t1 + 8);
  uVar3 = *(ushort *)(in_t2 + 8);
  uVar4 = *(uint *)(param_2 + 0x10);
  param_1[3] = (uint)*(ushort *)(in_t0 + 8) + (uint)*(ushort *)(param_2 + 0xc) * 0x10000;
  param_1[5] = (uint)uVar2 + (uint)uVar1 * 0x10000;
  param_1[7] = (uint)uVar3;
  param_1[1] = uVar4;
  uVar4 = *(uint *)(in_t1 + 0x10);
  uVar5 = *(uint *)(in_t2 + 0x10);
  param_1[2] = *(uint *)(in_t0 + 0x10);
  param_1[4] = uVar4;
  param_1[6] = uVar5;
  uVar4 = **(uint **)(param_2 + 0x14);
  **(uint **)(param_2 + 0x14) = (uint)param_1 & 0xffffff;
  *param_1 = uVar4 | 0x7000000;
  return;
}

