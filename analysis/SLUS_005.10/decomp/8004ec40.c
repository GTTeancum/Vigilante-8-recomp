// addr: 0x8004ec40  name: DIVGT3A_OBJ_40C

void DIVGT3A_OBJ_40C(uint *param_1,int param_2)

{
  undefined1 uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  int in_t0;
  int in_t1;
  int in_t2;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  
  uVar2 = *(ushort *)(param_2 + 0xe);
  uVar3 = *(ushort *)(in_t1 + 8);
  uVar4 = *(ushort *)(in_t2 + 8);
  uVar1 = *(undefined1 *)(param_2 + 0x13);
  param_1[3] = (uint)*(ushort *)(in_t0 + 8) + (uint)*(ushort *)(param_2 + 0xc) * 0x10000;
  param_1[6] = (uint)uVar3 + (uint)uVar2 * 0x10000;
  param_1[9] = (uint)uVar4;
  *(undefined1 *)(in_t0 + 0xf) = uVar1;
  uVar5 = *(uint *)(in_t1 + 0x10);
  uVar6 = *(uint *)(in_t2 + 0x10);
  param_1[2] = *(uint *)(in_t0 + 0x10);
  param_1[5] = uVar5;
  param_1[8] = uVar6;
  uVar7 = **(uint **)(param_2 + 0x14);
  **(uint **)(param_2 + 0x14) = (uint)param_1 & 0xffffff;
  uVar5 = *(uint *)(in_t1 + 0xc);
  uVar6 = *(uint *)(in_t2 + 0xc);
  param_1[1] = *(uint *)(in_t0 + 0xc);
  param_1[4] = uVar5;
  param_1[7] = uVar6;
  *param_1 = uVar7 | 0x9000000;
  return;
}

